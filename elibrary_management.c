/*
    E-Library Management System
    Corrected and commented learning version.

    Files used:
      books.dat    -> book records
      issues.dat   -> issue / due / return records
      accounts.dat -> student and librarian accounts

    Compile (GCC):
      gcc elibrary.c -o elibrary

    Windows:
      elibrary.exe

    Linux/macOS:
      ./elibrary

    Note: This is a simple educational file-based system. Passwords are stored
    as plain text; do not use this design for a real production application.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
    #include <conio.h>
    #define CLEAR_SCREEN() system("cls")
    #define PAUSE() getch()
#else
    #define CLEAR_SCREEN() system("clear")
    #define PAUSE() do { printf("\nPress Enter to continue..."); getchar(); } while (0)
#endif

#define BOOK_FILE "books.dat"
#define ISSUE_FILE "issues.dat"
#define ACCOUNT_FILE "accounts.dat"

#define MAX_BOOKS 500
#define MAX_ISSUES 1000
#define MAX_ACCOUNTS 500

#define USERNAME_SIZE 30
#define PASSWORD_SIZE 20
#define BOOK_NAME_SIZE 50
#define CATEGORY_SIZE 30
#define STUDENT_ID_SIZE 50

/* A date is stored as day, month and year. */
typedef struct {
    int yyyy;
    int mm;
    int dd;
} Date;

/* Information about a book in the library. */
typedef struct {
    int isbn;
    char bookName[BOOK_NAME_SIZE];
    int total_copy;
    int available;
    int book_status;          /* 1 = active, 0 = deleted */
    char category[CATEGORY_SIZE];
    int edition;
} Book;

/* Each issue record tracks one book loan. */
typedef struct {
    int isbn;
    char studentID[STUDENT_ID_SIZE];
    Date Issue_date;
    Date due_date;
    Date return_date;         /* all zero means book has not been returned */
} BookIssue;

/* Account type: 1 = librarian, 2 = student. */
typedef struct {
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    int type_account;
    int status;               /* 1 = active, 0 = disabled */
} Person;

/* Function prototypes: allow functions to call menus defined later. */
void menu_student(void);
void menu_librarian(void);
void acc_menu(void);
void acc_login(void);
void create_account(void);
void add_books(void);
void search_books(void);
void search_category(void);
void search_menu(void);
void view_books(void);
void edit_books(void);
void delete_books(void);
void issue_books(void);
void return_books(void);
void view_issue(void);

/* ---------- General input helpers ---------- */

/* Read a complete line safely, including spaces, and remove its newline. */
void read_line(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    /* If the line was longer than the buffer, discard the remaining input. */
    if (strchr(buffer, '\n') == NULL) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) { }
    }
}

/* Read an integer and reject non-numeric input. */
int read_int(const char *prompt) {
    char input[100];
    char *end;
    long value;

    while (1) {
        read_line(prompt, input, sizeof(input));
        value = strtol(input, &end, 10);

        if (input[0] != '\0' && *end == '\0') {
            return (int)value;
        }
        printf("Invalid input. Please enter an integer.\n");
    }
}

/* Compare strings without case sensitivity (portable alternative to strcmpi). */
int equal_ignore_case(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

/* Print a centered heading. */
void printMessageCenter(const char *message) {
    int i;
    int spaces = (78 - (int)strlen(message)) / 2;
    if (spaces < 0) spaces = 0;

    printf("\t\t\t");
    for (i = 0; i < spaces; i++) printf(" ");
    printf("%s\n", message);
}

/* Display common program heading. */
void headMessage(const char *message) {
    CLEAR_SCREEN();
    printf("\n\t\t\t#########################################################################");
    printf("\n\t\t\t############                                                 ############");
    printf("\n\t\t\t############       E-Library Management System               ############");
    printf("\n\t\t\t############                                                 ############");
    printf("\n\t\t\t#########################################################################");
    printf("\n\t\t\t---------------------------------------------------------------------------\n");
    printMessageCenter(message);
    printf("\t\t\t----------------------------------------------------------------------------\n");
}

/* ---------- Date functions ---------- */

/* Check whether a year is leap year. */
int is_leap_year(int year) {
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

/* Return number of days in a month. */
int days_in_month(int month, int year) {
    int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month < 1 || month > 12) return 0;
    if (month == 2 && is_leap_year(year)) return 29;
    return days[month];
}

/* Validate a date entered by the user. */
int valid_date(Date date) {
    if (date.yyyy < 1 || date.mm < 1 || date.mm > 12) return 0;
    if (date.dd < 1 || date.dd > days_in_month(date.mm, date.yyyy)) return 0;
    return 1;
}

/* Ask for a date until a valid dd/mm/yyyy date is entered. */
Date input_date(const char *prompt) {
    Date date;
    char line[100];

    while (1) {
        read_line(prompt, line, sizeof(line));
        if (sscanf(line, "%d/%d/%d", &date.dd, &date.mm, &date.yyyy) == 3
            && valid_date(date)) {
            return date;
        }
        printf("Invalid date. Use a valid date in dd/mm/yyyy format.\n");
    }
}

/* Add a number of days to a date. Handles month/year and leap years. */
Date add_days(Date date, int amount) {
    while (amount > 0) {
        date.dd++;
        if (date.dd > days_in_month(date.mm, date.yyyy)) {
            date.dd = 1;
            date.mm++;
            if (date.mm > 12) {
                date.mm = 1;
                date.yyyy++;
            }
        }
        amount--;
    }
    return date;
}

/* Print a date. */
void print_date(Date date) {
    if (date.dd == 0 && date.mm == 0 && date.yyyy == 0)
        printf("Not returned");
    else
        printf("%02d/%02d/%04d", date.dd, date.mm, date.yyyy);
}

/* ---------- File loading helpers ---------- */

/* Load all book records from file into an array. */
int load_books(Book books[]) {
    FILE *file = fopen(BOOK_FILE, "rb");
    int count = 0;

    if (file == NULL) return 0; /* File may not exist yet. */

    while (count < MAX_BOOKS &&
           fread(&books[count], sizeof(Book), 1, file) == 1) {
        count++;
    }

    fclose(file);
    return count;
}

/* Load all issue records from file into an array. */
int load_issues(BookIssue issues[]) {
    FILE *file = fopen(ISSUE_FILE, "rb");
    int count = 0;

    if (file == NULL) return 0;

    while (count < MAX_ISSUES &&
           fread(&issues[count], sizeof(BookIssue), 1, file) == 1) {
        count++;
    }

    fclose(file);
    return count;
}

/* Save all book records, replacing the previous file. */
int save_books(Book books[], int count) {
    FILE *file = fopen(BOOK_FILE, "wb");
    if (file == NULL) {
        printf("Could not open books file for writing.\n");
        return 0;
    }

    int written = (int)fwrite(books, sizeof(Book), count, file);
    fclose(file);
    return written == count;
}

/* Save all issue records, replacing the previous file. */
int save_issues(BookIssue issues[], int count) {
    FILE *file = fopen(ISSUE_FILE, "wb");
    if (file == NULL) {
        printf("Could not open issue file for writing.\n");
        return 0;
    }

    int written = (int)fwrite(issues, sizeof(BookIssue), count, file);
    fclose(file);
    return written == count;
}

/* Find a book by ISBN. Returns array index or -1 if not found. */
int find_book_by_isbn(Book books[], int count, int isbn) {
    int i;
    for (i = 0; i < count; i++) {
        if (books[i].isbn == isbn && books[i].book_status)
            return i;
    }
    return -1;
}

/* Display one book record neatly. */
void display_book(Book book) {
    printf("\nISBN / Book ID       : %d", book.isbn);
    printf("\nBook Name            : %s", book.bookName);
    printf("\nCategory             : %s", book.category);
    printf("\nEdition              : %d", book.edition);
    printf("\nTotal Copies         : %d", book.total_copy);
    printf("\nAvailable Copies     : %d\n", book.available);
}

/* ---------- Book management functions ---------- */

/* Add a new book record to the library. */
void add_books(void) {
    Book books[MAX_BOOKS];
    Book newBook;
    int count = load_books(books);

    headMessage("ADD BOOK");

    if (count >= MAX_BOOKS) {
        printf("Book storage limit reached.\n");
        PAUSE();
        return;
    }

    newBook.isbn = read_int("Enter ISBN (1-100000): ");
    if (newBook.isbn < 1 || newBook.isbn > 100000 ||
        find_book_by_isbn(books, count, newBook.isbn) != -1) {
        printf("Invalid or already-existing ISBN.\n");
        PAUSE();
        return;
    }

    read_line("Enter Book Name: ", newBook.bookName, sizeof(newBook.bookName));
    if (newBook.bookName[0] == '\0') {
        printf("Book name cannot be empty.\n");
        PAUSE();
        return;
    }

    newBook.total_copy = read_int("Enter total copies (1-100): ");
    if (newBook.total_copy < 1 || newBook.total_copy > 100) {
        printf("Copies must be between 1 and 100.\n");
        PAUSE();
        return;
    }

    read_line("Enter Category: ", newBook.category, sizeof(newBook.category));
    newBook.edition = read_int("Enter Edition (positive number): ");
    if (newBook.edition < 1 || newBook.edition > 99) {
        printf("Invalid edition.\n");
        PAUSE();
        return;
    }

    newBook.available = newBook.total_copy;
    newBook.book_status = 1;

    books[count] = newBook;
    if (save_books(books, count + 1))
        printf("\nBook added successfully!\n");
    else
        printf("\nError saving book record.\n");

    PAUSE();
}

/* Search books by name. Matching is case-insensitive. */
void search_books(void) {
    int i;
    Book books[MAX_BOOKS];
    int count = load_books(books);
    char searchName[BOOK_NAME_SIZE];
    int found = 0;

    headMessage("SEARCH BY BOOK NAME");
    read_line("Enter book name: ", searchName, sizeof(searchName));

    for (i = 0; i < count; i++) {
        if (books[i].book_status &&
            equal_ignore_case(searchName, books[i].bookName)) {
            display_book(books[i]);
            found = 1;
        }
    }

    if (!found) printf("\nNo matching book found.\n");
    PAUSE();
}

/* Search books by category (not by book name). */
void search_category(void) {
    int i;
    Book books[MAX_BOOKS];
    int count = load_books(books);
    char searchCategory[CATEGORY_SIZE];
    int found = 0;

    headMessage("SEARCH BY CATEGORY");
    read_line("Enter category: ", searchCategory, sizeof(searchCategory));

    for (i = 0; i < count; i++) {
        if (books[i].book_status &&
            equal_ignore_case(searchCategory, books[i].category)) {
            display_book(books[i]);
            found = 1;
        }
    }

    if (!found) printf("\nNo books found in this category.\n");
    PAUSE();
}

/* Menu to select a search method. */
void search_menu(void) {
    int choice;

    do {
        headMessage("SEARCH BOOKS MENU");
        printf("\n1. Search by Book Name");
        printf("\n2. Search by Category");
        printf("\n0. Back");
        choice = read_int("\nEnter choice: ");

        switch (choice) {
            case 1: search_books(); break;
            case 2: search_category(); break;
            case 0: break;
            default:
                printf("Invalid choice.\n");
                PAUSE();
        }
    } while (choice != 0);
}

/* Display every active book. */
void view_books(void) {
    int i;
    Book books[MAX_BOOKS];
    int count = load_books(books);
    int found = 0;

    headMessage("VIEW BOOKS");

    for (i = 0; i < count; i++) {
        if (books[i].book_status) {
            printf("\n--- Book %d ---", i + 1);
            display_book(books[i]);
            found = 1;
        }
    }

    if (!found) printf("\nNo books currently exist in the library.\n");
    PAUSE();
}

/* Edit the category or edition of a book selected by ISBN. */
void edit_books(void) {
    Book books[MAX_BOOKS];
    int count = load_books(books);
    int isbn, index, choice;

    headMessage("EDIT BOOK");
    isbn = read_int("Enter ISBN of the book to edit: ");
    index = find_book_by_isbn(books, count, isbn);

    if (index == -1) {
        printf("Book not found.\n");
        PAUSE();
        return;
    }

    display_book(books[index]);

    do {
        printf("\n\n1. Edit Category");
        printf("\n2. Edit Edition");
        printf("\n3. Finish editing");
        choice = read_int("\nEnter choice: ");

        switch (choice) {
            case 1:
                read_line("Enter new category: ",
                          books[index].category, sizeof(books[index].category));
                break;
            case 2: {
                int edition = read_int("Enter new edition (1-99): ");
                if (edition >= 1 && edition <= 99)
                    books[index].edition = edition;
                else
                    printf("Edition is invalid.\n");
                break;
            }
            case 3:
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while (choice != 3);

    if (save_books(books, count))
        printf("\nBook updated successfully.\n");
    else
        printf("\nCould not save changes.\n");

    PAUSE();
}

/* Soft-delete a book: keep its record but mark it inactive.
   Do not delete a book if any loan of it is still outstanding. */
void delete_books(void) {
    int i;
    Book books[MAX_BOOKS];
    BookIssue issues[MAX_ISSUES];
    int bookCount = load_books(books);
    int issueCount = load_issues(issues);
    int isbn, index, hasOutstanding = 0;

    headMessage("DELETE BOOK");
    isbn = read_int("Enter ISBN to delete: ");
    index = find_book_by_isbn(books, bookCount, isbn);

    if (index == -1) {
        printf("Book not found.\n");
        PAUSE();
        return;
    }

    for (i = 0; i < issueCount; i++) {
        if (issues[i].isbn == isbn &&
            issues[i].return_date.dd == 0) {
            hasOutstanding = 1;
            break;
        }
    }

    if (hasOutstanding) {
        printf("This book has an outstanding loan and cannot be deleted.\n");
        PAUSE();
        return;
    }

    display_book(books[index]);
    printf("\nAre you sure? (1 = Yes, 0 = No): ");
    int confirm = read_int("");

    if (confirm == 1) {
        books[index].book_status = 0;
        if (save_books(books, bookCount))
            printf("Book marked as deleted.\n");
        else
            printf("Could not save changes.\n");
    } else {
        printf("Deletion cancelled.\n");
    }

    PAUSE();
}

/* ---------- Issue and return functions ---------- */

/* Issue a book to a student and reduce available copies by one. */
void issue_books(void) {
    int i;
    Book books[MAX_BOOKS];
    BookIssue issues[MAX_ISSUES];
    int bookCount = load_books(books);
    int issueCount = load_issues(issues);
    char studentID[STUDENT_ID_SIZE];
    int isbn, index;

    headMessage("ISSUE BOOK");

    if (issueCount >= MAX_ISSUES) {
        printf("Issue-record storage limit reached.\n");
        PAUSE();
        return;
    }

    isbn = read_int("Enter ISBN of book: ");
    index = find_book_by_isbn(books, bookCount, isbn);

    if (index == -1) {
        printf("Book not found.\n");
        PAUSE();
        return;
    }

    if (books[index].available <= 0) {
        printf("No copies are currently available.\n");
        PAUSE();
        return;
    }

    read_line("Enter Student ID: ", studentID, sizeof(studentID));
    if (studentID[0] == '\0') {
        printf("Student ID cannot be empty.\n");
        PAUSE();
        return;
    }

    /* Prevent the same student from borrowing this ISBN twice at once. */
    for (i = 0; i < issueCount; i++) {
        if (issues[i].isbn == isbn &&
            equal_ignore_case(issues[i].studentID, studentID) &&
            issues[i].return_date.dd == 0) {
            printf("This student already has this book issued.\n");
            PAUSE();
            return;
        }
    }

    issues[issueCount].isbn = isbn;
    strcpy(issues[issueCount].studentID, studentID);
    issues[issueCount].Issue_date = input_date("Issue date (dd/mm/yyyy): ");
    issues[issueCount].due_date = add_days(issues[issueCount].Issue_date, 20);
    issues[issueCount].return_date.dd = 0;
    issues[issueCount].return_date.mm = 0;
    issues[issueCount].return_date.yyyy = 0;

    books[index].available--;

    if (!save_books(books, bookCount)) {
        printf("Error saving book availability.\n");
        PAUSE();
        return;
    }

    if (!save_issues(issues, issueCount + 1)) {
        /* Try to restore the copy count if issue file saving failed. */
        books[index].available++;
        save_books(books, bookCount);
        printf("Error saving issue record. Book availability restored.\n");
        PAUSE();
        return;
    }

    printf("\nBook issued successfully!");
    printf("\nDue date (20 days after issue): ");
    print_date(issues[issueCount].due_date);
    printf("\n");
    PAUSE();
}

/* Return a book using student ID and ISBN.
   Only the matching unreturned issue record is updated. */
void return_books(void) {
    int i;
    Book books[MAX_BOOKS];
    BookIssue issues[MAX_ISSUES];
    int bookCount = load_books(books);
    int issueCount = load_issues(issues);
    char studentID[STUDENT_ID_SIZE];
    int isbn, issueIndex = -1, bookIndex;

    headMessage("RETURN BOOK");

    read_line("Enter Student ID: ", studentID, sizeof(studentID));
    isbn = read_int("Enter ISBN: ");

    /* Find the student's outstanding loan for this particular book. */
    for (i = 0; i < issueCount; i++) {
        if (issues[i].isbn == isbn &&
            equal_ignore_case(issues[i].studentID, studentID) &&
            issues[i].return_date.dd == 0) {
            issueIndex = i;
            break;
        }
    }

    if (issueIndex == -1) {
        printf("\nNo outstanding issue record found for this student/book.\n");
        PAUSE();
        return;
    }

    bookIndex = find_book_by_isbn(books, bookCount, isbn);
    if (bookIndex == -1) {
        printf("Book record is missing or inactive.\n");
        PAUSE();
        return;
    }

    issues[issueIndex].return_date = input_date("Return date (dd/mm/yyyy): ");
    books[bookIndex].available++;

    if (books[bookIndex].available > books[bookIndex].total_copy)
        books[bookIndex].available = books[bookIndex].total_copy;

    /* Save both files. If book save fails, do not proceed. */
    if (!save_books(books, bookCount)) {
        printf("Could not update book availability.\n");
        PAUSE();
        return;
    }

    if (!save_issues(issues, issueCount)) {
        /* Roll back availability as a best-effort recovery. */
        books[bookIndex].available--;
        save_books(books, bookCount);
        printf("Could not save return record; availability restored.\n");
        PAUSE();
        return;
    }

    printf("\nBook returned successfully.");
    printf("\nDue date: ");
    print_date(issues[issueIndex].due_date);
    printf("\nReturned on: ");
    print_date(issues[issueIndex].return_date);

    /* Compare full dates, not just day number. */
    if (issues[issueIndex].return_date.yyyy < issues[issueIndex].due_date.yyyy ||
        (issues[issueIndex].return_date.yyyy == issues[issueIndex].due_date.yyyy &&
         issues[issueIndex].return_date.mm < issues[issueIndex].due_date.mm) ||
        (issues[issueIndex].return_date.yyyy == issues[issueIndex].due_date.yyyy &&
         issues[issueIndex].return_date.mm == issues[issueIndex].due_date.mm &&
         issues[issueIndex].return_date.dd <= issues[issueIndex].due_date.dd)) {
        printf("\nReturned on time.\n");
    } else {
        printf("\nReturned late.\n");
    }

    PAUSE();
}

/* Display all issue records, including returned and pending books. */
void view_issue(void) {
    int i;
    BookIssue issues[MAX_ISSUES];
    int count = load_issues(issues);

    headMessage("VIEW ISSUED BOOKS");

    if (count == 0) {
        printf("No issue records found.\n");
        PAUSE();
        return;
    }

    for (i = 0; i < count; i++) {
        printf("\n--- Issue Record %d ---", i + 1);
        printf("\nISBN             : %d", issues[i].isbn);
        printf("\nStudent ID       : %s", issues[i].studentID);
        printf("\nIssue date       : ");
        print_date(issues[i].Issue_date);
        printf("\nDue date         : ");
        print_date(issues[i].due_date);
        printf("\nReturn date      : ");
        print_date(issues[i].return_date);
        printf("\nStatus           : %s\n",
               issues[i].return_date.dd == 0 ? "Pending" : "Returned");
    }

    PAUSE();
}

/* ---------- Account functions ---------- */

/* Create an account. Username must be unique. */
void create_account(void) {
    int i;
    Person accounts[MAX_ACCOUNTS];
    Person newAccount;
    FILE *file;
    int count = 0, choice;

    headMessage("CREATE ACCOUNT");

    file = fopen(ACCOUNT_FILE, "rb");
    if (file != NULL) {
        while (count < MAX_ACCOUNTS &&
               fread(&accounts[count], sizeof(Person), 1, file) == 1) {
            count++;
        }
        fclose(file);
    }

    if (count >= MAX_ACCOUNTS) {
        printf("Account storage limit reached.\n");
        PAUSE();
        return;
    }

    read_line("Enter username: ", newAccount.username, sizeof(newAccount.username));
    if (newAccount.username[0] == '\0') {
        printf("Username cannot be empty.\n");
        PAUSE();
        return;
    }

    for (i = 0; i < count; i++) {
        if (equal_ignore_case(accounts[i].username, newAccount.username)) {
            printf("Username already exists.\n");
            PAUSE();
            return;
        }
    }

    read_line("Enter password: ", newAccount.password, sizeof(newAccount.password));
    if (newAccount.password[0] == '\0') {
        printf("Password cannot be empty.\n");
        PAUSE();
        return;
    }

    printf("\n1. Librarian\n2. Student\n");
    choice = read_int("Choose account type: ");
    if (choice != 1 && choice != 2) {
        printf("Invalid account type.\n");
        PAUSE();
        return;
    }

    newAccount.type_account = choice;
    newAccount.status = 1;
    accounts[count] = newAccount;

    file = fopen(ACCOUNT_FILE, "wb");
    if (file == NULL) {
        printf("Could not open accounts file.\n");
        PAUSE();
        return;
    }

    int written = (int)fwrite(accounts, sizeof(Person), count, file);
    int newWritten = (int)fwrite(&newAccount, sizeof(Person), 1, file);
    fclose(file);

    if (written == count && newWritten == 1)
        printf("\nAccount created successfully.\n");
    else
        printf("\nError writing account data.\n");

    PAUSE();
}

/* Login and open the correct menu based on account type. */
void acc_login(void) {
    Person account;
    FILE *file;
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    int loggedIn = 0;

    headMessage("ACCOUNT LOGIN");
    read_line("Username: ", username, sizeof(username));
    read_line("Password: ", password, sizeof(password));

    file = fopen(ACCOUNT_FILE, "rb");
    if (file == NULL) {
        printf("\nNo account file exists yet. Create an account first.\n");
        PAUSE();
        return;
    }

    while (fread(&account, sizeof(Person), 1, file) == 1) {
        if (account.status &&
            equal_ignore_case(account.username, username) &&
            strcmp(account.password, password) == 0) {
            loggedIn = 1;
            printf("\nLogin successful!\n");

            time_t now = time(NULL);
            printf("Login time: %s", ctime(&now));
            PAUSE();

            if (account.type_account == 1)
                menu_librarian();
            else if (account.type_account == 2)
                menu_student();

            break;
        }
    }

    fclose(file);

    if (!loggedIn) {
        printf("\nLogin failed. Username or password is incorrect.\n");
        PAUSE();
    }
}

/* ---------- Main menus ---------- */

/* Student menu: students can search and view books only. */
void menu_student(void) {
    int choice;

    do {
        headMessage("STUDENT MAIN MENU");
        printf("\n1. Search Books");
        printf("\n2. View Books");
        printf("\n0. Logout");
        choice = read_int("\nEnter choice: ");

        switch (choice) {
            case 1: search_menu(); break;
            case 2: view_books(); break;
            case 0: printf("Logging out...\n"); break;
            default:
                printf("Invalid choice.\n");
                PAUSE();
        }
    } while (choice != 0);
}

/* Librarian menu: librarian manages the books and loans. */
void menu_librarian(void) {
    int choice;

    do {
        headMessage("LIBRARIAN MAIN MENU");
        printf("\n1. Add Book");
        printf("\n2. Search Book");
        printf("\n3. View Books");
        printf("\n4. Edit Book");
        printf("\n5. Delete Book");
        printf("\n6. Issue Book");
        printf("\n7. Return Book");
        printf("\n8. View Issued Books");
        printf("\n0. Logout");
        choice = read_int("\nEnter choice: ");

        switch (choice) {
            case 1: add_books(); break;
            case 2: search_menu(); break;
            case 3: view_books(); break;
            case 4: edit_books(); break;
            case 5: delete_books(); break;
            case 6: issue_books(); break;
            case 7: return_books(); break;
            case 8: view_issue(); break;
            case 0: printf("Logging out...\n"); break;
            default:
                printf("Invalid choice.\n");
                PAUSE();
        }
    } while (choice != 0);
}

/* Initial portal menu: login, signup, or exit the application. */
void acc_menu(void) {
    int choice;

    do {
        headMessage("WELCOME TO LOGIN / SIGN-UP PORTAL");
        printf("\n1. Login");
        printf("\n2. Sign-Up");
        printf("\n0. Exit");
        choice = read_int("\nEnter choice: ");

        switch (choice) {
            case 1: acc_login(); break;
            case 2: create_account(); break;
            case 0:
                printf("\nThank you for using E-Library.\n");
                break;
            default:
                printf("Invalid choice.\n");
                PAUSE();
        }
    } while (choice != 0);
}

/* Program execution begins here. */
int main(void) {
    acc_menu();
    return 0;
}
