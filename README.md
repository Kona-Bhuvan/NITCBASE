# NITCbase: A Layered Relational Database Management System

**NITCbase** is a pedagogical RDBMS developed to understand the internal workings of database engines. This project implements an 8-layer architecture, abstracting operations from raw disk I/O to high-level SQL command processing.

## 🏗️ Project Architecture

The system is built on a modular, layered design where each component communicates with the layers directly above and below it:

### 1. **Mynitcbase (Core Engine)**
This is the heartbeat of the system, containing the implementation of the primary database logic:
* **Frontend & Interface:** Handles user interaction and command parsing.
* **Algebra Layer:** Executes relational algebra operations (Select, Project, Join).
* **Schema Layer:** Manages Data Definition Language (DDL) tasks like creating/dropping tables.
* **B+ Tree & Block Access:** Handles efficient data indexing ($O(\log n)$ search) and record-level storage.
* **Buffer & Cache:** Manages in-memory data structures and an LRU-based buffer pool to optimize disk access.

### 2. **XFS Interface**
A specialized interface layer that bridges the internal database structures with the host filesystem, allowing for external command execution and data movement.

### 3. **Disk Emulator**
Located in the `/Disk` directory, this simulates the physical hardware. It manages the `disk.dat` file, providing a controlled environment to test low-level block reads and writes.

---

### 📂 Directory Structure

```text
NITCbase
├── Disk/                       # Disk Emulator and control tools
│   ├── disk                    # Simulated disk file
│   └── disk_run_copy           # Run copy of the disk
├── Files/                      # Data files for testing and execution
│   ├── Batch_Execution_Files/  # Batch scripts (e.g., s8test.txt, s11test.txt)
│   ├── Input_Files/            # CSV datasets for relation loading
│   └── Output_Files/           # Resultant CSVs from queries and catalogs
├── mynitcbase/                 # Core RDBMS Implementation
│   ├── Algebra/                # Relational Algebra (Select, Project, Join)
│   ├── BPlusTree/              # Indexing structures
│   ├── BlockAccess/            # Record-level disk block operations
│   ├── Buffer/                 # Buffer management (StaticBuffer, BlockBuffer)
│   ├── Cache/                  # Relational and Attribute caching
│   ├── Disk_Class/             # Low-level disk interaction
│   ├── Frontend/               # Command parsing
│   ├── FrontendInterface/      # Interface logic and Regex handlers
│   ├── Schema/                 # DDL operations (Create, Drop)
│   └── main.cpp                # Entry point
├── XFS_Interface/              # Interface for external filesystem commands
├── other/                      # Documentation, PDFs, and Dockerfile
└── Dockerfile                  # Containerization configuration
```

---

## 🛠️ Technical Highlights

* **Language:** C++
* **Indexing:** B+ Tree implementation for fast record retrieval.
* **Memory Management:** Custom **Buffer Management** system to minimize hardware bottleneck using an **LRU** policy.
* **Relational Algebra:** Implementation of Equi-Join, Selection, and Projection.
* **Containerization:** Includes a `Dockerfile` for a consistent, platform-independent build environment.

---

## 🚦 Getting Started

### Prerequisites
* GCC/G++ Compiler
* Make build system
* Linux-based environment (or Docker)

### Build and Run
1.  **Clone the repository:**
    ```bash
    git clone https://github.com/Kona-Bhuvan/NITCBASE.git
    cd NITCBASE/mynitcbase
    ```
2.  **Compile the project:**
    ```bash
    make
    ```
3.  **Launch the database:**
    ```bash
    ./nitcbase
    ```

---

## 🐳 Running with Docker

This project is containerized to ensure a consistent environment across different machines. 

### 1. Build the Image
From the root directory, run:
```bash
docker build -t nitcbase .
```

### 2. Run the Container
Launch the database engine in interactive mode:
```bash
docker run -it nitcbase
```

---

## 📖 Acknowledgments
Developed as part of the Database Management Systems laboratory course at the **National Institute of Technology Calicut**.
Guide: https://nitcbase.github.io/
