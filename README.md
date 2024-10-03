# C-Server

C-Server is a simple C web server application that can serve HTML files. It is a work in progress and may never be fully completed. The server is designed to run on Linux environments.

## Features

- Serve static HTML files.
- Simple routing for basic web pages.
- Easy to extend and modify for personal needs.

## Getting Started

### Prerequisites

Make sure you have the following installed:

- A C compiler (e.g., `gcc`)
- `make` utility for building the project
- `clang-format` for code formatting

### Installing the compiler

To compile the program, you need to install the `C standard library` and `gcc` on your system:

#### Ubuntu/Debian compiler

```bash
sudo apt-get install libc6-dev
sudo apt-get install gcc
```

#### Fedora compiler

```bash
sudo dnf install glibc-devel
sudo dnf install gcc
```

### Installing make

To build the project, you need the make utility:

#### Ubuntu/Debian make

```bash
sudo apt-get install make
```

#### Fedora make

```bash
sudo dnf install make
```

### Installing the Formatter

To ensure consistent code formatting, install `clang-format` on your system:

#### Ubuntu/Debian Formatter

```bash
sudo apt-get install clang-format
```

#### Fedora Formatter

```bash
sudo sudo dnf install clang-format
```

### Cloning the Repository

#### Clone the repository to your local machine

```bash
git clone https://github.com/yourusername/c-server.git
cd c-server
```

### Building the Project

#### To compile the project, navigate to the project directory and run

```bash
make
```

This command will generate an executable named web_server.

### Running the Server

#### To run the server, use the following command

```bash
./web_server
```

By default, the server will run on port 8080. You can access it by navigating to http://localhost:8080 in your web browser.

### Code Formatting

#### To format the code, you can run

```bash
make format
```

This will apply clang-format to the source files for consistent styling