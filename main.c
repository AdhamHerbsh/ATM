#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

// --- Constants ---
#define ACCOUNTS_FILE "accounts.csv"
#define MAX_ACCOUNTS 10
#define CSV_BUFFER_SIZE 1024
#define CSV_LINE_SIZE 256

// --- Structures ---
typedef struct {
    int accountNumber;
    char pin[5]; // 4-digit PIN + null terminator
    double balance;
    char name[50];
} Account;

typedef struct {
    // Main window widgets
    GtkWidget *window;
    GtkWidget *stack;
    GtkWidget *main_box;

    // Welcome screen widgets
    GtkWidget *welcome_box;
    GtkWidget *create_account_button;
    GtkWidget *login_button;
    GtkWidget *exit_button;

    // Login screen widgets
    GtkWidget *login_box;
    GtkWidget *account_entry;
    GtkWidget *pin_entry;
    GtkWidget *login_submit_button;
    GtkWidget *login_back_button;
    GtkWidget *login_status_label;

    // Create account screen widgets
    GtkWidget *create_box;
    GtkWidget *name_entry;
    GtkWidget *new_pin_entry;
    GtkWidget *confirm_pin_entry;
    GtkWidget *create_submit_button;
    GtkWidget *create_back_button;
    GtkWidget *create_status_label;

    // User dashboard widgets
    GtkWidget *dashboard_box;
    GtkWidget *balance_label;
    GtkWidget *welcome_label;
    GtkWidget *withdraw_button;
    GtkWidget *deposit_button;
    GtkWidget *logout_button;

    // Transaction widgets
    GtkWidget *transaction_box;
    GtkWidget *amount_entry;
    GtkWidget *transaction_submit_button;
    GtkWidget *transaction_cancel_button;
    GtkWidget *transaction_status_label;

    // State variables
    char transaction_type; // 'W' for withdraw, 'D' for deposit

    // Account information
    Account accounts[MAX_ACCOUNTS];
    int numAccounts;
    Account *currentAccount;
} ATMApp;

// --- Function Prototypes ---
// CSV Handling
static char* escape_csv_field(const char* field);
static char* unescape_csv_field(const char* field);
static int split_csv_line(char* line, char** fields, int max_fields);

// Account Management
static void load_accounts(ATMApp *app);
static void save_accounts(ATMApp *app);
static int find_account(ATMApp *app, int account_number);
static bool verify_pin(Account *account, const char *pin);
static bool is_valid_pin(const char *pin);

// UI Navigation
static void show_screen(ATMApp *app, const char *screen_name);
static void update_balance_display(ATMApp *app);
static gboolean delayed_show_screen(gpointer user_data);

// Signal Handlers
static void on_exit_clicked(GtkButton *button, gpointer user_data);
static void on_login_button_clicked(GtkButton *button, gpointer user_data);
static void on_create_account_button_clicked(GtkButton *button, gpointer user_data);
static void on_login_submit_clicked(GtkButton *button, gpointer user_data);
static void on_create_submit_clicked(GtkButton *button, gpointer user_data);
static void on_back_button_clicked(GtkButton *button, gpointer user_data);
static void on_logout_clicked(GtkButton *button, gpointer user_data);
static void on_withdraw_clicked(GtkButton *button, gpointer user_data);
static void on_deposit_clicked(GtkButton *button, gpointer user_data);
static void on_transaction_submit_clicked(GtkButton *button, gpointer user_data);
static void on_transaction_cancel_clicked(GtkButton *button, gpointer user_data);

// --- CSV Handling Functions ---

// Escapes special characters in CSV fields (commas, quotes, newlines)
static char* escape_csv_field(const char* field) {
    if (field == NULL) return NULL;

    // Count characters that need escaping
    int escaped_count = 0;
    bool needs_quotes = false;
    for (int i = 0; field[i]; i++) {
        if (field[i] == '"') {
            escaped_count++;  // Double quotes need to be doubled
        }
        if (field[i] == ',' || field[i] == '"' || field[i] == '\n' || field[i] == '\r') {
            needs_quotes = true;
        }
    }

    // If no special characters, return a simple copy
    if (!needs_quotes && escaped_count == 0) {
        return strdup(field);
    }

    // Allocate memory for escaped string
    size_t length = strlen(field);
    char* escaped = (char*)malloc(length + escaped_count + 3); // +2 for quotes, +1 for null terminator
    if (escaped == NULL) return NULL;

    // Add opening quote
    int pos = 0;
    escaped[pos++] = '"';

    // Copy and escape the field
    for (int i = 0; field[i]; i++) {
        if (field[i] == '"') {
            escaped[pos++] = '"';  // Double the quote
        }
        escaped[pos++] = field[i];
    }

    // Add closing quote and null terminator
    escaped[pos++] = '"';
    escaped[pos] = '\0';

    return escaped;
}

// Unescapes CSV field (removes quotes and handles doubled quotes)
static char* unescape_csv_field(const char* field) {
    if (field == NULL) return NULL;

    size_t len = strlen(field);
    if (len < 2) return strdup(field); // Not quoted

    // Check if the field is quoted
    if (field[0] == '"' && field[len-1] == '"') {
        // Allocate memory for unescaped field
        char* unescaped = (char*)malloc(len - 1); // -2 for quotes, +1 for null terminator
        if (unescaped == NULL) return NULL;

        int pos = 0;
        // Copy and unescape the content (skip first and last quotes)
        for (size_t i = 1; i < len - 1; i++) {
            if (field[i] == '"' && field[i+1] == '"') {
                unescaped[pos++] = '"';
                i++; // Skip the second quote
            } else {
                unescaped[pos++] = field[i];
            }
        }
        unescaped[pos] = '\0';
        return unescaped;
    } else {
        // Not quoted, just duplicate
        return strdup(field);
    }
}

// Splits a CSV line into fields
static int split_csv_line(char* line, char** fields, int max_fields) {
    int field_count = 0;
    int i = 0;
    bool in_quotes = false;
    char* start = line;

    while (line[i] != '\0' && field_count < max_fields) {
        if (line[i] == '"') {
            in_quotes = !in_quotes;
        } else if (line[i] == ',' && !in_quotes) {
            // End of field
            line[i] = '\0';
            fields[field_count++] = start;
            start = &line[i + 1];
        }
        i++;
    }

    // Add the last field if there's room
    if (field_count < max_fields && start[0] != '\0') {
        fields[field_count++] = start;
    }

    return field_count;
}

// --- Account Management Functions ---

// Loads account data from the CSV file
static void load_accounts(ATMApp *app) {
    FILE *file = fopen(ACCOUNTS_FILE, "r");
    if (file == NULL) {
        g_print("No existing accounts file found. Starting fresh.\n");
        app->numAccounts = 0;

        // Create a new file with header
        file = fopen(ACCOUNTS_FILE, "w");
        if (file != NULL) {
            fprintf(file, "AccountNumber,PIN,Balance,Name\n");
            fclose(file);
        }
        return;
    }

    char buffer[CSV_BUFFER_SIZE];
    int line_count = 0;
    app->numAccounts = 0;

    // Read the file line by line
    while (fgets(buffer, CSV_BUFFER_SIZE, file) != NULL) {
        // Skip header line
        if (line_count == 0) {
            line_count++;
            continue;
        }

        // Remove newline character if present
        size_t len = strlen(buffer);
        if (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r')) {
            buffer[len-1] = '\0';
        }
        if (len > 1 && buffer[len-2] == '\r') {
            buffer[len-2] = '\0';
        }

        // Split the line into fields
        char* fields[4] = {NULL};
        int field_count = split_csv_line(buffer, fields, 4);

        if (field_count == 4) {
            // Parse account data
            Account *account = &app->accounts[app->numAccounts];
            account->accountNumber = atoi(fields[0]);

            char* unescaped_pin = unescape_csv_field(fields[1]);
            strncpy(account->pin, unescaped_pin, 4);
            account->pin[4] = '\0';
            free(unescaped_pin);

            account->balance = atof(fields[2]);

            char* unescaped_name = unescape_csv_field(fields[3]);
            strncpy(account->name, unescaped_name, 49);
            account->name[49] = '\0';
            free(unescaped_name);

            app->numAccounts++;
            if (app->numAccounts >= MAX_ACCOUNTS) {
                g_print("Maximum number of accounts reached.\n");
                break;
            }
        }

        line_count++;
    }

    fclose(file);
    g_print("Accounts loaded successfully (%d accounts).\n", app->numAccounts);
}

// Saves account data to the CSV file
static void save_accounts(ATMApp *app) {
    FILE *file = fopen(ACCOUNTS_FILE, "w");
    if (file == NULL) {
        g_print("Error opening accounts file for saving.\n");
        return;
    }

    // Write header
    fprintf(file, "AccountNumber,PIN,Balance,Name\n");

    // Write each account
    for (int i = 0; i < app->numAccounts; i++) {
        Account *account = &app->accounts[i];

        // Escape the name field to handle commas and quotes
        char* escaped_name = escape_csv_field(account->name);

        // Write the account data
        fprintf(file, "%d,%s,%.2f,%s\n",
                account->accountNumber,
                account->pin,
                account->balance,
                escaped_name);

        free(escaped_name);
    }

    fclose(file);
    g_print("Accounts saved successfully.\n");
}

// Finds an account by account number
static int find_account(ATMApp *app, int account_number) {
    for (int i = 0; i < app->numAccounts; i++) {
        if (app->accounts[i].accountNumber == account_number) {
            return i; // Return the index of the account
        }
    }
    return -1; // Account not found
}

// Verifies the PIN for a given account
static bool verify_pin(Account *account, const char *pin) {
    return (strcmp(account->pin, pin) == 0);
}

// Validates that a PIN contains exactly 4 digits
static bool is_valid_pin(const char *pin) {
    if (strlen(pin) != 4) {
        return false;
    }

    for (int i = 0; i < 4; i++) {
        if (!isdigit(pin[i])) {
            return false;
        }
    }

    return true;
}

// --- UI Navigation Functions ---

// Shows a specific screen in the stack
static void show_screen(ATMApp *app, const char *screen_name) {
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), screen_name);
}

// Updates the balance display on the dashboard
static void update_balance_display(ATMApp *app) {
    if (app->currentAccount) {
        char balance_text[100];
        sprintf(balance_text, "Current Balance: $%.2f", app->currentAccount->balance);
        gtk_label_set_text(GTK_LABEL(app->balance_label), balance_text);

        char welcome_text[100];
        sprintf(welcome_text, "Welcome, %s!", app->currentAccount->name);
        gtk_label_set_text(GTK_LABEL(app->welcome_label), welcome_text);
    }
}

// Callback for delayed screen transition
static gboolean delayed_show_screen(gpointer user_data) {
    ATMApp *app = (ATMApp *)user_data;
    const char *screen_name = g_object_get_data(G_OBJECT(app->window), "screen_name");
    show_screen(app, screen_name);
    return G_SOURCE_REMOVE;  // Don't call again
}

// --- Signal Handlers Implementation ---

static void on_exit_clicked(GtkButton *button, gpointer user_data) {
    ATMApp *app = (ATMApp *)user_data;
    save_accounts(app);
    gtk_window_close(GTK_WINDOW(app->window));
}

static void on_login_button_clicked(GtkButton *button, gpointer user_data) {
    ATMApp *app = (ATMApp *)user_data;
    // Clear previous entries and status
    gtk_editable_set_text(GTK_EDITABLE(app->account_entry), "");
    gtk_editable_set_text(GTK_EDITABLE(app->pin_entry), "");
    gtk_label_set_text(GTK_LABEL(app->login_status_label), "");
    show_screen(app, "login");
}

static void on_create_account_button_clicked(GtkButton *button, gpointer user_data) {
    ATMApp *app = (ATMApp *)user_data;
    // Clear previous entries and status
    gtk_editable_set_text(GTK_EDITABLE(app->name_entry), "");
    gtk_editable_set_text(GTK_EDITABLE(app->new_pin_entry), "");
    gtk_editable_set_text(GTK_EDITABLE(app->confirm_pin_entry), "");
    gtk_label_set_text(GTK_LABEL(app->create_status_label), "");
    show_screen(app, "create");
}

static void on_login_submit_clicked(GtkButton *button, gpointer user_data) {
    ATMApp *app = (ATMApp *)user_data;

    const char *account_str = gtk_editable_get_text(GTK_EDITABLE(app->account_entry));
    const char *pin = gtk_editable_get_text(GTK_EDITABLE(app->pin_entry));

    // Validate input
    if (strlen(account_str) == 0 || strlen(pin) == 0) {
        gtk_label_set_text(GTK_LABEL(app->login_status_label), "Please enter account number and PIN");
        return;
    }

    int account_number = atoi(account_str);
    int index = find_account(app, account_number);

    if (index != -1 && verify_pin(&app->accounts[index], pin)) {
        app->currentAccount = &app->accounts[index];
        update_balance_display(app);
        show_screen(app, "dashboard");
    } else {
        gtk_label_set_text(GTK_LABEL(app->login_status_label), "Invalid account number or PIN");
    }
}

static void on_create_submit_clicked(GtkButton *button, gpointer user_data) {
    ATMApp *app = (ATMApp *)user_data;

    const char *name = gtk_editable_get_text(GTK_EDITABLE(app->name_entry));
    const char *pin = gtk_editable_get_text(GTK_EDITABLE(app->new_pin_entry));
    const char *confirm_pin = gtk_editable_get_text(GTK_EDITABLE(app->confirm_pin_entry));

    // Validate input
    if (strlen(name) == 0) {
        gtk_label_set_text(GTK_LABEL(app->create_status_label), "Please enter your name");
        return;
    }

    if (!is_valid_pin(pin)) {
        gtk_label_set_text(GTK_LABEL(app->create_status_label), "PIN must be 4 digits");
        return;
    }

    if (strcmp(pin, confirm_pin) != 0) {
        gtk_label_set_text(GTK_LABEL(app->create_status_label), "PINs do not match");
        return;
    }

    if (app->numAccounts >= MAX_ACCOUNTS) {
        gtk_label_set_text(GTK_LABEL(app->create_status_label), "Cannot create more accounts. Maximum limit reached.");
        return;
    }

    // Create new account
    Account newAccount;
    strncpy(newAccount.name, name, 49);
    newAccount.name[49] = '\0'; // Ensure null termination
    strcpy(newAccount.pin, pin);
    newAccount.accountNumber = 1000 + app->numAccounts;
    newAccount.balance = 0.0;

    app->accounts[app->numAccounts++] = newAccount;
    save_accounts(app);

    char success_msg[100];
    sprintf(success_msg, "Account created successfully! Your account number is %d", newAccount.accountNumber);
    gtk_label_set_text(GTK_LABEL(app->create_status_label), success_msg);
}

static void on_back_button_clicked(GtkButton *button, gpointer user_data) {
    ATMApp *app = (ATMApp *)user_data;
    show_screen(app, "welcome");
}

static void on_logout_clicked(GtkButton *button, gpointer user_data) {
    ATMApp *app = (ATMApp *)user_data;
    app->currentAccount = NULL;
    show_screen(app, "welcome");
}

static void on_withdraw_clicked(GtkButton *button, gpointer user_data) {
    ATMApp *app = (ATMApp *)user_data;
    app->transaction_type = 'W';
    gtk_label_set_text(GTK_LABEL(app->transaction_status_label), "Enter amount to withdraw:");
    gtk_editable_set_text(GTK_EDITABLE(app->amount_entry), "");
    show_screen(app, "transaction");
}

static void on_deposit_clicked(GtkButton *button, gpointer user_data) {
    ATMApp *app = (ATMApp *)user_data;
    app->transaction_type = 'D';
    gtk_label_set_text(GTK_LABEL(app->transaction_status_label), "Enter amount to deposit:");
    gtk_editable_set_text(GTK_EDITABLE(app->amount_entry), "");
    show_screen(app, "transaction");
}

static void on_transaction_submit_clicked(GtkButton *button, gpointer user_data) {
    ATMApp *app = (ATMApp *)user_data;

    const char *amount_str = gtk_editable_get_text(GTK_EDITABLE(app->amount_entry));
    double amount = atof(amount_str);

    if (amount <= 0) {
        gtk_label_set_text(GTK_LABEL(app->transaction_status_label), "Please enter a valid amount");
        return;
    }

    if (app->transaction_type == 'W') {
        // Withdraw
        if (amount > app->currentAccount->balance) {
            gtk_label_set_text(GTK_LABEL(app->transaction_status_label), "Insufficient funds");
            return;
        }

        app->currentAccount->balance -= amount;
        gtk_label_set_text(GTK_LABEL(app->transaction_status_label), "Withdrawal successful");
    } else {
        // Deposit
        app->currentAccount->balance += amount;
        gtk_label_set_text(GTK_LABEL(app->transaction_status_label), "Deposit successful");
    }

    save_accounts(app);
    update_balance_display(app);

    // Return to dashboard after a short delay
    g_object_set_data(G_OBJECT(app->window), "screen_name", "dashboard");
    g_timeout_add_seconds(1, delayed_show_screen, app);
}

static void on_transaction_cancel_clicked(GtkButton *button, gpointer user_data) {
    ATMApp *app = (ATMApp *)user_data;
    show_screen(app, "dashboard");
}

// --- Main Application Function ---
static void activate(GtkApplication *gtk_app, gpointer user_data) {
    // Create and initialize ATM application
    ATMApp *app = g_new0(ATMApp, 1);
    app->currentAccount = NULL;

    // Load accounts
    load_accounts(app);

    // Create main window
    app->window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(app->window), "ATM Simulator");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 400, 500);

    // Create CSS provider for styling
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "button { padding: 10px; margin: 5px; min-width: 120px; }"
        "label.title { font-size: 24px; font-weight: bold; margin: 15px; }"
        "label.balance { font-size: 18px; margin: 10px; }"
        "label.error { color: red; }"
        "label.success { color: green; }"
        "entry { margin: 5px; padding: 5px; min-width: 200px; }"
        "box { padding: 10px; }",
        -1);
    gtk_style_context_add_provider_for_display(
        gtk_widget_get_display(app->window),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Main container
    app->main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(app->window), app->main_box);

    // Create stack for different screens
    app->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(app->stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_box_append(GTK_BOX(app->main_box), app->stack);

    // --- Welcome Screen ---
    app->welcome_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(app->welcome_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(app->welcome_box, GTK_ALIGN_CENTER);
    gtk_stack_add_named(GTK_STACK(app->stack), app->welcome_box, "welcome");

    GtkWidget *welcome_label = gtk_label_new("Welcome to ATM Simulator");
    gtk_widget_add_css_class(welcome_label, "title");
    gtk_box_append(GTK_BOX(app->welcome_box), welcome_label);

    app->create_account_button = gtk_button_new_with_label("Create Account");
    gtk_box_append(GTK_BOX(app->welcome_box), app->create_account_button);
    g_signal_connect(app->create_account_button, "clicked", G_CALLBACK(on_create_account_button_clicked), app);

    app->login_button = gtk_button_new_with_label("Login");
    gtk_box_append(GTK_BOX(app->welcome_box), app->login_button);
    g_signal_connect(app->login_button, "clicked", G_CALLBACK(on_login_button_clicked), app);

    app->exit_button = gtk_button_new_with_label("Exit");
    gtk_box_append(GTK_BOX(app->welcome_box), app->exit_button);
    g_signal_connect(app->exit_button, "clicked", G_CALLBACK(on_exit_clicked), app);

    // --- Login Screen ---
    app->login_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(app->login_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(app->login_box, GTK_ALIGN_CENTER);
    gtk_stack_add_named(GTK_STACK(app->stack), app->login_box, "login");

    GtkWidget *login_title = gtk_label_new("Login to Your Account");
    gtk_widget_add_css_class(login_title, "title");
    gtk_box_append(GTK_BOX(app->login_box), login_title);

    GtkWidget *account_label = gtk_label_new("Account Number:");
    gtk_box_append(GTK_BOX(app->login_box), account_label);

    app->account_entry = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(app->account_entry), GTK_INPUT_PURPOSE_DIGITS);
    gtk_box_append(GTK_BOX(app->login_box), app->account_entry);

    GtkWidget *pin_label = gtk_label_new("PIN:");
    gtk_box_append(GTK_BOX(app->login_box), pin_label);

    app->pin_entry = gtk_password_entry_new();
    gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(app->pin_entry), TRUE);
    gtk_entry_set_max_length(GTK_ENTRY(app->pin_entry), 4);
    gtk_entry_set_input_purpose(GTK_ENTRY(app->pin_entry), GTK_INPUT_PURPOSE_PIN);
    gtk_box_append(GTK_BOX(app->login_box), app->pin_entry);

    app->login_status_label = gtk_label_new("");
    gtk_widget_add_css_class(app->login_status_label, "error");
    gtk_box_append(GTK_BOX(app->login_box), app->login_status_label);

    GtkWidget *login_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(login_button_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(app->login_box), login_button_box);

    app->login_submit_button = gtk_button_new_with_label("Login");
    gtk_box_append(GTK_BOX(login_button_box), app->login_submit_button);
    g_signal_connect(app->login_submit_button, "clicked", G_CALLBACK(on_login_submit_clicked), app);

    app->login_back_button = gtk_button_new_with_label("Back");
    gtk_box_append(GTK_BOX(login_button_box), app->login_back_button);
    g_signal_connect(app->login_back_button, "clicked", G_CALLBACK(on_back_button_clicked), app);

    // --- Create Account Screen ---
    app->create_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(app->create_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(app->create_box, GTK_ALIGN_CENTER);
    gtk_stack_add_named(GTK_STACK(app->stack), app->create_box, "create");

    GtkWidget *create_title = gtk_label_new("Create New Account");
    gtk_widget_add_css_class(create_title, "title");
    gtk_box_append(GTK_BOX(app->create_box), create_title);

    GtkWidget *name_label = gtk_label_new("Your Name:");
    gtk_box_append(GTK_BOX(app->create_box), name_label);

    app->name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->name_entry), "Enter your full name");
    gtk_box_append(GTK_BOX(app->create_box), app->name_entry);

    GtkWidget *new_pin_label = gtk_label_new("Create PIN (4 digits):");
    gtk_box_append(GTK_BOX(app->create_box), new_pin_label);

    app->new_pin_entry = gtk_password_entry_new();
    gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(app->new_pin_entry), TRUE);
    gtk_entry_set_max_length(GTK_ENTRY(app->new_pin_entry), 4);
    gtk_entry_set_input_purpose(GTK_ENTRY(app->new_pin_entry), GTK_INPUT_PURPOSE_PIN);
    gtk_box_append(GTK_BOX(app->create_box), app->new_pin_entry);

    GtkWidget *confirm_pin_label = gtk_label_new("Confirm PIN:");
    gtk_box_append(GTK_BOX(app->create_box), confirm_pin_label);

    app->confirm_pin_entry = gtk_password_entry_new();
    gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(app->confirm_pin_entry), TRUE);
    gtk_entry_set_max_length(GTK_ENTRY(app->confirm_pin_entry), 4);
    gtk_entry_set_input_purpose(GTK_ENTRY(app->confirm_pin_entry), GTK_INPUT_PURPOSE_PIN);
    gtk_box_append(GTK_BOX(app->create_box), app->confirm_pin_entry);

    app->create_status_label = gtk_label_new("");
    gtk_widget_add_css_class(app->create_status_label, "success");
    gtk_box_append(GTK_BOX(app->create_box), app->create_status_label);

    GtkWidget *create_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(create_button_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(app->create_box), create_button_box);

    app->create_submit_button = gtk_button_new_with_label("Create Account");
    gtk_box_append(GTK_BOX(create_button_box), app->create_submit_button);
    g_signal_connect(app->create_submit_button, "clicked", G_CALLBACK(on_create_submit_clicked), app);

    app->create_back_button = gtk_button_new_with_label("Back");
    gtk_box_append(GTK_BOX(create_button_box), app->create_back_button);
    g_signal_connect(app->create_back_button, "clicked", G_CALLBACK(on_back_button_clicked), app);

    // --- Dashboard Screen ---
    app->dashboard_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(app->dashboard_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(app->dashboard_box, GTK_ALIGN_CENTER);
    gtk_stack_add_named(GTK_STACK(app->stack), app->dashboard_box, "dashboard");

    app->welcome_label = gtk_label_new("");
    gtk_widget_add_css_class(app->welcome_label, "title");
    gtk_box_append(GTK_BOX(app->dashboard_box), app->welcome_label);

    app->balance_label = gtk_label_new("");
    gtk_widget_add_css_class(app->balance_label, "balance");
    gtk_box_append(GTK_BOX(app->dashboard_box), app->balance_label);

    app->withdraw_button = gtk_button_new_with_label("Withdraw");
    gtk_box_append(GTK_BOX(app->dashboard_box), app->withdraw_button);
    g_signal_connect(app->withdraw_button, "clicked", G_CALLBACK(on_withdraw_clicked), app);

    app->deposit_button = gtk_button_new_with_label("Deposit");
    gtk_box_append(GTK_BOX(app->dashboard_box), app->deposit_button);
    g_signal_connect(app->deposit_button, "clicked", G_CALLBACK(on_deposit_clicked), app);

    app->logout_button = gtk_button_new_with_label("Logout");
    gtk_box_append(GTK_BOX(app->dashboard_box), app->logout_button);
    g_signal_connect(app->logout_button, "clicked", G_CALLBACK(on_logout_clicked), app);

    // --- Transaction Screen ---
    app->transaction_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(app->transaction_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(app->transaction_box, GTK_ALIGN_CENTER);
    gtk_stack_add_named(GTK_STACK(app->stack), app->transaction_box, "transaction");

    GtkWidget *transaction_title = gtk_label_new("Transaction");
    gtk_widget_add_css_class(transaction_title, "title");
    gtk_box_append(GTK_BOX(app->transaction_box), transaction_title);

    app->transaction_status_label = gtk_label_new("");
    gtk_widget_add_css_class(app->transaction_status_label, "success");
    gtk_box_append(GTK_BOX(app->transaction_box), app->transaction_status_label);

    GtkWidget *amount_label = gtk_label_new("Amount:");
    gtk_box_append(GTK_BOX(app->transaction_box), amount_label);

    app->amount_entry = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(app->amount_entry), GTK_INPUT_PURPOSE_NUMBER);
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->amount_entry), "Enter amount");
    gtk_box_append(GTK_BOX(app->transaction_box), app->amount_entry);

    GtkWidget *transaction_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(transaction_button_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(app->transaction_box), transaction_button_box);

    app->transaction_submit_button = gtk_button_new_with_label("Submit");
    gtk_box_append(GTK_BOX(transaction_button_box), app->transaction_submit_button);
    g_signal_connect(app->transaction_submit_button, "clicked", G_CALLBACK(on_transaction_submit_clicked), app);

    app->transaction_cancel_button = gtk_button_new_with_label("Cancel");
    gtk_box_append(GTK_BOX(transaction_button_box), app->transaction_cancel_button);
    g_signal_connect(app->transaction_cancel_button, "clicked", G_CALLBACK(on_transaction_cancel_clicked), app);

    // Show welcome screen
    show_screen(app, "welcome");

    // Show the window
    gtk_window_present(GTK_WINDOW(app->window));
}

int main(int argc, char *argv[]) {
    GtkApplication *gtk_app = gtk_application_new("com.example.atm_simulator", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(gtk_app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(gtk_app), argc, argv);
    g_object_unref(gtk_app);
    return status;
}
