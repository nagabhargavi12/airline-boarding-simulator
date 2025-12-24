# Airline Boarding Management System

This is a console-based Airline Boarding Management System developed in C.
The project simulates real-world airline operations including flight
management, passenger handling, check-in, and priority-based boarding.

## Features
- Role-based login for Staff, Gate Agent, and Cabin Crew
- Add and manage flights with status (ONTIME / DELAYED)
- Add passengers and assign them to flights
- Passenger check-in system
- Priority-based boarding using Heap (VIP, Elderly, Business)
- Normal boarding using Queue (Economy – FIFO)
- Linked list to store final boarded passengers
- Daily boarding logs stored in date-wise files
- Special assistance handling (wheelchair)

## Concepts Used
- C Structures
- File Handling
- Data Structures:
  - Heap (priority boarding)
  - Queue (normal boarding)
  - Linked List (boarded passengers)
- Role-based authentication
- Menu-driven programming

## Files Used
- airline.c  : Main source code
- staff.txt : Stores login credentials and roles
- boarding_*.txt : Daily boarding log files (auto-generated)

## How to Compile and Run
Compile:
gcc airline.c -o airline

Run:
./airline

## Login Credentials Format
The staff.txt file should contain entries in the following format:
username password ROLE

Example:
staff staff123 STAFF
agent agent123 AGENT
crew crew123 CREW

## Note
The daily boarding log files (boarding_*.txt) are generated automatically
at runtime. These files are ignored in GitHub using .gitignore to avoid
uploading runtime-generated data.
