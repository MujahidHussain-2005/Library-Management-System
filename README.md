# E-Library Management System (C)

A console-based **E-Library Management System** written in C with role-based menus for **Librarians** and **Students**, and binary/raw struct file persistence (`account.txt`, `book.txt`, and `issue.txt`).

---

## Features

- **Account Portal (`acc_menu`, `create_account`, `acc_login`)**
  - Sign up as a **Librarian** (`type_account = 1`) or **Student** (`type_account = 2`).
  - Log in with username and password verification, including login/logout timestamps via `<time.h>`.
- **Librarian Menu (`menu_librarian`)**
  - **Add Book (`add_books`)**: Add ISBN, book name, total/available copies, category, and edition.
  - **Search Book (`search_menu`, `search_books`, `search_category`)**: Search books by name or category.
  - **View Books (`view_books`)**: Display active books and copy availability.
  - **Edit Book (`edit_books`)**: Update an existing book's category or edition.
  - **Delete Book (`delete_books`)**: Soft-delete a book by setting `book_status = 0`.
  - **Issue Book (`issue_books`)**: Record a book issue with student ID and issue date, automatically calculate a 20-day due date, and decrement available copies.
  - **Return Book (`return_books`)**: Record a return date, compare against the due date, and increment available book copies.
  - **View Issued Books (`view_issue`)**: View issued book records and dates.
- **Student Menu (`menu_student`)**
  - Search books (by name or category) and view available books.

---

## Project Structure & Data Files

| File / Component | Description |
| :--- | :--- |
| `main.c` (your source file) | Contains all menu logic, struct definitions (`Date`, `books`, `books_issue`, `person`), and file operations. |
| `account.txt` | Stores registered user records (`struct person`). |
| `book.txt` | Stores catalog entries (`struct books`). |
| `issue.txt` | Stores book borrowing and return records (`struct books_issue`). |

---

## Requirements & Compilation

This code uses Windows console/compiler-specific headers and functions such as `<conio.h>` (`getch()`), `system("cls")`, and `strcmpi()`.

### Windows (MinGW / MSVC)

Using **GCC (MinGW)**:

```bash
gcc library.c -o library.exe
./library.exe
