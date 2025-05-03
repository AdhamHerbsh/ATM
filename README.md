# ATM Simulator Project Document

## Introduction

The ATM Simulator is a desktop application designed to emulate the core functionalities of an Automated Teller Machine (ATM). It allows users to create accounts, log in, check balances, deposit funds, and withdraw money, with data persistently stored in a CSV file. Built with GTK 4 for a modern graphical interface, the application provides a user-friendly experience while demonstrating robust data handling and secure PIN verification.

### Project Background

- **Purpose**: To create a functional ATM simulation for educational and demonstration purposes, showcasing GUI programming and file-based data persistence.
- **Scope**: Supports account creation, login, balance inquiries, deposits, and withdrawals for up to 10 accounts, with data saved in `accounts.csv`.

### Motivation

- **Learning Objective**: To understand GUI development with GTK 4, CSV file handling, and secure data management.
- **Practical Application**: Provides a simplified model of real-world ATM systems, useful for teaching banking system concepts.

## Project Overview

The ATM Simulator replicates essential banking operations through a desktop application. Users interact via a graphical interface to perform transactions, with account data stored in a CSV file for persistence across sessions. The project emphasizes usability, data integrity, and modularity.

### Application Features

- **Account Management**: Create and manage up to 10 accounts with unique account numbers and PINs.
- **Transactions**: Support for deposits and withdrawals with balance validation.
- **Data Persistence**: Store account details (account number, PIN, balance, name) in `accounts.csv`.
- **Security**: PIN-based authentication for account access.

### Flow Chart

- **Process Overview**:
    - Start → Welcome Screen → [User Choice: Login, Create Account, Exit]
    - Login → Enter Account Number & PIN → Validate → Dashboard → [Withdraw, Deposit, Logout]
    - Create Account → Enter Name & PIN → Validate → Save to CSV → Display Account Number
    - Exit → Save Accounts → Close Application
- **Diagram**: A flow chart would depict the decision tree from the welcome screen, branching to login, account creation, or exit, with sub-branches for dashboard actions and transaction outcomes.

## Application Definition

The ATM Simulator is a standalone desktop application that mimics ATM operations. It uses GTK 4 for the user interface and C for backend logic, with account data stored in a CSV file. The application is designed for single-user interaction, simulating a personal banking experience.

### Functional Requirements

- **Account Creation**: Users can create an account with a name and 4-digit PIN, assigned a unique account number.
- **Login**: Users log in using their account number and PIN.
- **Transactions**: Users can deposit or withdraw funds, with balance checks to prevent overdrafts.
- **Data Storage**: Account details are saved in `accounts.csv` with proper CSV escaping for special characters.

### Non-Functional Requirements

- **Usability**: Intuitive interface with clear navigation and error messages.
- **Performance**: Fast response times for UI interactions and file operations.
- **Reliability**: Robust CSV parsing to handle malformed data gracefully.

### User Flow

- **Navigation Path**:
    - Welcome Screen: Displays buttons for Login, Create Account, or Exit.
    - Login Screen: Input account number and PIN; success leads to Dashboard, failure shows an error.
    - Create Account Screen: Input name and PIN (confirmed); success saves account and shows account number.
    - Dashboard: Shows balance and offers Withdraw, Deposit, or Logout options.
    - Transaction Screen: Input amount for deposit/withdrawal; success updates balance, failure shows an error.
- **User Flow Diagram**: Illustrates the sequence of screens and user inputs, with decision points (e.g., valid PIN, sufficient balance) and transitions (e.g., back to Welcome after Logout).

## Objectives

The ATM Simulator aims to provide a functional, user-friendly banking simulation while demonstrating key software development concepts.

### Primary Goals

- **Simulate ATM Operations**: Accurately replicate account creation, login, and transaction processes.
- **Ensure Data Persistence**: Save and load account data reliably using CSV.
- **Provide Intuitive UI**: Design a clear, responsive interface using GTK 4.

### Secondary Goals

- **Enhance Security**: Implement PIN verification and safe data handling.
- **Promote Modularity**: Structure code for easy maintenance and future enhancements.
- **Handle Edge Cases**: Manage special characters in names and malformed CSV data.

### Success Metrics

- **Functionality**: All features (account creation, login, transactions) work as intended.
- **Usability**: Users can navigate and complete tasks without confusion.
- **Data Integrity**: CSV file accurately reflects account states after operations.

## Technologies Used

The ATM Simulator leverages modern technologies to achieve its functionality and user experience.

### Programming Language

- **C**: Used for backend logic, including CSV file operations and account management.
- **Why Chosen**: Offers low-level control for file handling and integrates well with GTK 4.

### GUI Framework

- **GTK 4**: Provides the graphical interface with widgets like buttons, entries, and labels.
- **Why Chosen**: Cross-platform, modern, and supports complex UI designs with CSS styling.

### Data Storage

- **CSV File (`accounts.csv`)**: Stores account data in a human-readable format.
- **Why Chosen**: Simple, portable, and suitable for small-scale data storage.

### Development Tools

- **GCC**: Compiler for building the C application.
- **pkg-config**: Manages GTK 4 library dependencies.
- **Valgrind**: Used for memory leak detection during testing.

### Technology Flow

- **Interaction Flow**: User inputs → GTK 4 UI → C logic → CSV read/write → UI feedback.
- **Diagram**: A technology flow chart would show GTK 4 rendering the UI, C processing user inputs, and file I/O operations updating `accounts.csv`.

## How It Works

The ATM Simulator operates through a series of interconnected components that handle user interactions, data processing, and storage.

### Core Functionality

- **Account Creation**: Users enter a name and 4-digit PIN; the system assigns an account number (1000 + index) and saves to CSV.
- **Login**: Users enter an account number and PIN; the system verifies against CSV data and grants access to the dashboard.
- **Transactions**: Users select Deposit or Withdraw, enter an amount, and the system updates the balance if valid (e.g., sufficient funds for withdrawal).
- **Data Persistence**: Account data is loaded from `accounts.csv` at startup and saved after each modification.

### Technical Workflow

- **Initialization**: Load accounts from `accounts.csv` into an array of `Account` structs.
- **UI Rendering**: GTK 4 displays screens (Welcome, Login, Create, Dashboard, Transaction) using a `GtkStack` for navigation.
- **User Input**: Signal handlers process button clicks and entry inputs, calling backend logic.
- **Data Handling**: CSV functions (`load_accounts`, `save_accounts`, `escape_csv_field`, etc.) manage file I/O with proper escaping for special characters.
- **Feedback**: UI updates with success/error messages and balance displays.

### Process Flow Chart

- **Workflow**:
    - Start: Load CSV → Display Welcome Screen.
    - User Action: Select option → Route to appropriate screen.
    - Login: Validate credentials → Show Dashboard or error.
    - Create: Validate input → Save to CSV → Show success.
    - Transaction: Validate amount → Update balance → Save to CSV → Return to Dashboard.
    - Exit: Save CSV → Close.
- **Diagram**: A flow chart would map the sequence of user actions, backend processing, and CSV updates, with decision points for validation.

## User Interface Guide

The ATM Simulator features a clean, intuitive interface built with GTK 4, styled via CSS for visual consistency.

### Screen Descriptions

- **Welcome Screen**:
    - Displays "Welcome to ATM" with buttons: Create Account, Login, Exit.
    - **User Flow**: Click a button to navigate to the corresponding screen or exit.
- **Login Screen**:
    - Fields for account number and PIN, with Login and Back buttons.
    - **User Flow**: Enter credentials; success leads to Dashboard, failure shows an error, Back returns to Welcome.
- **Create Account Screen**:
    - Fields for name, PIN, and PIN confirmation, with Create and Back buttons.
    - **User Flow**: Enter valid data; success shows account number, failure shows an error, Back returns to Welcome.
- **Dashboard Screen**:
    - Shows welcome message, balance, and buttons: Withdraw, Deposit, Logout.
    - **User Flow**: Select transaction or logout; transactions lead to Transaction screen, Logout returns to Welcome.
- **Transaction Screen**:
    - Field for amount, with Submit and Cancel buttons.
    - **User Flow**: Enter amount; success updates balance and returns to Dashboard, failure shows an error, Cancel returns to Dashboard.

### UI Design Elements

- **CSS Styling**: Buttons (10px padding, 120px min-width), titles (24px bold), balance labels (18px), error (red), success (green), entries (200px width).
- **Layout**: Vertical `GtkBox` for each screen, centered with `GTK_ALIGN_CENTER`.

### User Flow Diagram

- **Navigation**:
    - Welcome → [Create → Create Screen] or [Login → Login Screen] or [Exit].
    - Login → [Valid → Dashboard] or [Invalid → Error] or [Back → Welcome].
    - Create → [Valid → Success Message] or [Invalid → Error] or [Back → Welcome].
    - Dashboard → [Withdraw/Deposit → Transaction] or [Logout → Welcome].
    - Transaction → [Valid → Dashboard] or [Invalid → Error] or [Cancel → Dashboard].
- **Diagram**: A user flow diagram would visualize screen transitions and user inputs, highlighting decision points (e.g., valid PIN, sufficient funds).

## Design

The ATM Simulator's design focuses on modularity, usability, and data integrity, with a clear separation of concerns.

### System Architecture

- **Frontend**: GTK 4 manages the UI, with a `GtkStack` for screen navigation and CSS for styling.
- **Backend**: C handles logic, including account validation, transaction processing, and CSV file operations.
- **Data Layer**: CSV file (`accounts.csv`) stores account data with a header (`AccountNumber,PIN,Balance,Name`).

### Component Diagram

- **Components**:
    - UI: `ATMApp` struct with widgets (`window`, `stack`, `buttons`, `entries`, `labels`).
    - Logic: Functions (`find_account`, `verify_pin`, `update_balance_display`).
    - Storage: CSV functions (`load_accounts`, `save_accounts`, `escape_csv_field`, etc.).
- **Diagram**: A component diagram would show the `ATMApp` interacting with GTK 4 for UI, C logic for processing, and CSV file for storage.

### Data Flow

- **Input**: User enters data via GTK entries (e.g., PIN, amount).
- **Processing**: C functions validate inputs and update `Account` structs.
- **Storage**: CSV functions read/write data to `accounts.csv`.
- **Output**: GTK labels display results (e.g., balance, error messages).
- **Data Flow Chart**: Would depict user input → validation → CSV update → UI feedback.

## Implementation

The ATM Simulator is implemented in C with GTK 4, following a structured approach to ensure reliability and maintainability.

### Code Structure

- **Main File (`atm.c`)**:
    - **Structures**: `Account` (account number, PIN, balance, name), `ATMApp` (widgets, state).
    - **Functions**: Signal handlers (`on_login_submit_clicked`, etc.), helpers (`load_accounts`, etc.).
    - **Main**: Initializes GTK application and activates the UI.
- **CSV Handling**:
    - `escape_csv_field`: Escapes commas, quotes, newlines in names.
    - `unescape_csv_field`: Reverses escaping for reading.
    - `split_csv_line`: Parses CSV lines into fields.
    - `load_accounts`: Reads `accounts.csv` into `Account` array.
    - `save_accounts`: Writes `Account` array to `accounts.csv`.

### Development Process

- **Design Phase**: Defined requirements, UI layout, and CSV format.
- **Implementation Phase**: Coded UI with GTK 4, backend logic, and CSV handling.
- **Testing Phase**: Tested account creation, login, transactions, and CSV integrity with special characters.
- **Debugging**: Used Valgrind for memory leaks and manual inspection of `accounts.csv`.

### Implementation Flow

- **Steps**:
    - Initialize GTK 4 and load CSV.
    - Set up `GtkStack` with screens and connect signals.
    - Handle user inputs via signal handlers, updating `Account` structs.
    - Save changes to CSV after account creation or transactions.
    - Clean up memory on exit.
- **Flow Chart**: Would illustrate the sequence of initialization, UI setup, user interaction, and cleanup, with branches for each screen's logic.

## Conclusion

The ATM Simulator successfully delivers a functional, user-friendly ATM emulation, demonstrating key concepts in GUI programming, data persistence, and secure data handling. Built with GTK 4 and C, it provides a robust platform for educational purposes and potential expansion.

### Achievements

- **Functional Success**: All core features (account creation, login, transactions) work reliably.
- **Usability**: Intuitive UI with clear feedback and navigation.
- **Data Integrity**: CSV storage handles special characters and persists data accurately.

### Future Enhancements

- **Additional Features**: Add balance inquiry, transaction history, or PIN change.
- **UI Improvements**: Integrate `GtkHeaderBar` or modern GTK 4 themes.
- **Security**: Encrypt PINs in CSV or use a database for scalability.
- **Validation**: Enhance input validation (e.g., restrict PIN to digits, validate name format).

### Final User Flow

- **Summary**: Users start at Welcome, create or log into accounts, perform transactions, and exit, with all data saved to CSV.
- **Diagram**: A final user flow diagram would encapsulate the entire user journey, from startup to exit, highlighting all possible paths and outcomes.
