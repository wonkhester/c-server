# C-Server

C-Server is a simple C web server application that can send HTML files. It's a work in progress and may never be finished. The server is designed to run on Linux and macOS environments.

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

### Installing the Formatter

To ensure consistent code formatting, install `clang-format` on your system:

#### Ubuntu/Debian

```bash
sudo apt-get install clang-format
```

#### Fedora

```bash
sudo apt-get install clang-format
```

#### macOS

```bash
sudo apt-get install clang-format
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