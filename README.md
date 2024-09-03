# DataLink

**Subtitle:** Command-Line Client-Server Database Utility


## 🚀 Overview

Welcome to **DataLink**—your go-to command-line tool for seamless client-server communication and database management. This project involves building a server that handles client connections, allowing clients to perform operations on a shared database file. The project is perfect for those looking to deepen their understanding of networking, file I/O, and system-level programming.

## 🛠 Features

- **Client-Server Architecture:** Efficient communication between client and server using sockets.
- **Database Operations:** Add, remove, update, and list employee records in a database file.
- **Error Handling:** Robust handling of invalid inputs and edge cases.
- **Memory Management:** Carefully designed to avoid memory leaks, with Valgrind verification.

## 📦 Installation

### Prerequisites

- **GCC** or any C compiler.
- **Make** (for using the provided Makefile).

### Build Instructions

## 1. Clone the repository:

   ```bash
   git clone https://github.com/Strongato/DataLink
   cd DataLink
   ```
   
## 2. Compile the Project

To compile the project, follow these steps:

1. **Ensure you have all the necessary development tools installed.** You can usually install these via your package manager. For example, on Ubuntu, you might run:

    ```bash
    sudo apt-get update
    sudo apt-get install build-essential
    ```

2. **Run `make` to compile the project**. This will generate the executables as specified in your `Makefile`:

    ```bash
    make
    ```

3. **Run `make clean`** if you need to clean up any generated files or start from scratch:

    ```bash
    make clean
    ```

## 📜 Usage


## 1. **How to Run the Server:**

- **`-h`**  - List all possible options and their use cases  
  **Usage:** `./bin/dbserver -h`

- **`-f`**  - (Required) Path to the database file  
  **Usage:** `./bin/dbserver -f <database file>`

- **`-p`**  - (Required) Set the port number  
  **Usage:** `./bin/dbserver -p <port>`  
  *(Port number must be in the range [1024, 65535])*

- **`-n`**  - Create a new database file  
  **Usage:** `./bin/dbserver -n`

### Examples:

1. **Create a new database file and start the server:**

    ```bash
    ./bin/dbserver -n -f mynewdb.db -p 8080
    ```

    This command starts the server on port `8080` and creates `mynewdb.db` as the database file.

2. **Open an existing database file and start the server:**

    ```bash
    ./bin/dbserver -f mynewdb.db -p 8080
    ```

    This command starts the server on port `8080` and opens `mynewdb.db` as the database file.

---

## 2. **Run the Client:**

- **`-h`**  - (Required) Host IP address  
  **Usage:** `./bin/dbcli -h <ip>`

- **`-p`**  - (Required) Set the port number  
  **Usage:** `./bin/dbcli -p <port>`

- **`-l`**  - List employees from the database file  
  **Usage:** `./bin/dbcli -l`

- **`-a`**  - Add an employee to the database file  
  **Usage:** `./bin/dbcli -a <name,address,hours>`

- **`-e`**  - Specify employee (required for `-r` and `-u` options)  
  **Usage:** `./bin/dbcli -e <name,address,hours>`

- **`-r`**  - Remove an employee from the database file (requires `-e` option)  
  **Usage:** `./bin/dbcli -r`

- **`-u`**  - Update an employee's hours worked (requires `-e` option)  
  **Usage:** `./bin/dbcli -u <hours>`

### Examples:

1. **Add an employee:**

    ```bash
    ./bin/dbcli -h 127.0.0.1 -p 8080 -a "Josip Milicic,Unicorn lane 3,40"
    ```

    This command adds an employee with the name "Josip Milicic", address "Unicorn lane 3", and 40 hours worked.

2. **List employees:**

    ```bash
    ./bin/dbcli -h 127.0.0.1 -p 8080 -l
    ```

    This command lists all employees in the database.

3. **Update an employee's hours worked:**

    ```bash
    ./bin/dbcli -h 127.0.0.1 -p 8080 -e "Josip Milicic,Unicorn lane 3,40" -u 100
    ```

    This command updates the hours worked for "Josip Milicic" at "Unicorn lane 3" to 100 hours.

4. **List employees again:**

    ```bash
    ./bin/dbcli -h 127.0.0.1 -p 8080 -l
    ```

    This command lists all employees after the update.

5. **Remove an employee:**

    ```bash
    ./bin/dbcli -h 127.0.0.1 -p 8080 -e "Josip Milicic,Unicorn lane 3,100" -r
    ```

    This command removes the employee "Josip Milicic" from "Unicorn lane 3" who has 100 hours worked.


## 🧪 Testing

To test the functionality of the server and client, follow these steps:

1. **Start the server** as described in the "Usage" section.
2. **Open another terminal** and run the client with various commands to test different functionalities (e.g., adding, updating, removing employees).
3. **Verify database integrity** list the database file with -l option.

## 📫 Contact

For any questions or support, please contact:

- **Josip Miličić** - josip.milicic37@gmail.com

---

Happy coding! 🎉
