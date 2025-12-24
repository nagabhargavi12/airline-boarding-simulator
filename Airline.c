// airline.c
// Compile: gcc airline.c -o airline
// Run: ./airline

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX 200

// ---------------- Flight Structure ----------------
typedef struct {
    char flightNo[16];
    char source[64];
    char destination[64];
    char deptTime[10];    // HH:MM
    char status[16];      // "ONTIME" or "DELAYED"
} Flight;

Flight flights[MAX];
int flightCount = 0;

// ---------------- Passenger Structure ----------------
typedef struct {
    char name[200];        // allow long multi-word full name
    int age;
    char ticketNo[40];
    char seat[12];
    char flightNo[16];     // links to Flight.flightNo
    int priority;          // VIP=10, Elderly=8, Business=6, Economy=3
    char assist[20];       // "none" or "wheelchair"
    int baggage;
    int isCheckedIn;       // 0/1
    int isBoarded;         // 0/1
} Passenger;

Passenger db[MAX];
int dbCount = 0;

// ---------------- Linked List for Boarding List --------------
typedef struct Node {
    Passenger p;
    struct Node *next;
} Node;
Node *boardHead = NULL;

// ---------------- Utility: get flight pointer ----------------
Flight* getFlightByNo(const char *fno) {
    for (int i = 0; i < flightCount; ++i) {
        if (strcmp(flights[i].flightNo, fno) == 0) return &flights[i];
    }
    return NULL;
}

// ---------------- Date and daily file ----------------
void getDate(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    sprintf(buffer, "%02d-%02d-%04d", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
}
void getDailyFileName(char *filename) {
    char date[20];
    getDate(date);
    sprintf(filename, "boarding_%s.txt", date);
}

void logBoardingToFile(Passenger p) {
    char filename[256];
    getDailyFileName(filename);

    FILE *f = fopen(filename, "a");
    if (!f) return;

    char dt[40];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    sprintf(dt, "%02d-%02d-%04d %02d:%02d:%02d",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec);

    Flight *fl = getFlightByNo(p.flightNo);
    if (fl) {
        fprintf(f, "%s | Age:%d | %s | Seat:%s | Flight:%s | %s->%s | Depart:%s | Status:%s | Priority:%d | Assist:%s | Time:%s\n",
                p.name, p.age, p.ticketNo, p.seat,
                fl->flightNo, fl->source, fl->destination,
                fl->deptTime, fl->status,
                p.priority, p.assist, dt);
    } else {
        fprintf(f, "%s | Age:%d | %s | Seat:%s | Flight:%s | Priority:%d | Assist:%s | Time:%s\n",
                p.name, p.age, p.ticketNo, p.seat, p.flightNo, p.priority, p.assist, dt);
    }

    fclose(f);
}

// ---------------- Authentication ----------------
int authenticate(char *u, char *p, char *roleOut) {
    FILE *f = fopen("staff.txt", "r");
    if (!f) return 0;
    char user[128], pass[128], role[64];
    while (fscanf(f, "%127s %127s %63s", user, pass, role) == 3) {
        if (!strcmp(u, user) && !strcmp(p, pass)) {
            strcpy(roleOut, role);
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

// ---------------- Linked list operations ----------------
void addBoarded(Passenger p) {
    Node *n = (Node*)malloc(sizeof(Node));
    if (!n) return;
    n->p = p;
    n->next = NULL;
    if (!boardHead) { boardHead = n; return; }
    Node *t = boardHead;
    while (t->next) t = t->next;
    t->next = n;
}
void showBoardedForFlight(const char *flightNo) {
    Node *t = boardHead;
    int found = 0;
    printf("\n=== FINAL BOARDING LIST for %s ===\n", flightNo);
    while (t) {
        if (strcmp(t->p.flightNo, flightNo) == 0) {
            Passenger p = t->p;
            printf("Name        : %s\n", p.name);
            printf("Age         : %d\n", p.age);
            printf("Ticket No   : %s\n", p.ticketNo);
            printf("Seat        : %s\n", p.seat);
            printf("Priority    : %d\n", p.priority);
            printf("Assistance  : %s\n", p.assist);
            printf("---------------------------------------\n");
            found = 1;
        }
        t = t->next;
    }
    if (!found) printf("No boarded passengers for flight %s yet.\n", flightNo);
}

// ---------------- Flight management ----------------
void addFlight() {
    Flight f;
    printf("\n--- Add Flight ---\n");
    printf("Flight No: ");
    scanf("%15s", f.flightNo);

    if (getFlightByNo(f.flightNo) != NULL) {
        printf("Flight already exists.\n");
        return;
    }

    // consume leftover newline before using fgets
    getchar();
    printf("Source: ");
    if (!fgets(f.source, sizeof(f.source), stdin)) f.source[0] = '\0';
    f.source[strcspn(f.source, "\n")] = '\0';

    printf("Destination: ");
    if (!fgets(f.destination, sizeof(f.destination), stdin)) f.destination[0] = '\0';
    f.destination[strcspn(f.destination, "\n")] = '\0';

    printf("Departure Time (HH:MM): ");
    scanf("%9s", f.deptTime);

    strcpy(f.status, "ONTIME");
    flights[flightCount++] = f;
    printf("Flight added successfully.\n");
}
void viewFlights() {
    if (flightCount == 0) { printf("No flights added yet.\n"); return; }
    printf("\n=== FLIGHTS ===\n");
    for (int i = 0; i < flightCount; ++i) {
        Flight *f = &flights[i];
        printf("Flight No  : %s\n", f->flightNo);
        printf("Route      : %s -> %s\n", f->source, f->destination);
        printf("Dep Time   : %s\n", f->deptTime);
        printf("Status     : %s\n", f->status);
        printf("---------------------------------------\n");
    }
}
void updateFlightDelay() {
    char fno[16];
    printf("Enter Flight No to update: ");
    scanf("%15s", fno);
    Flight *f = getFlightByNo(fno);
    if (!f) { printf("Flight not found.\n"); return; }
    printf("1. Mark DELAYED\n2. Mark ONTIME\nChoice: ");
    int ch; scanf("%d", &ch);
    if (ch == 1) strcpy(f->status, "DELAYED");
    else strcpy(f->status, "ONTIME");
    printf("Flight status updated.\n");
}

// ---------------- Passenger management ----------------
void addPassenger() {
    if (flightCount == 0) { printf("Add at least one flight first.\n"); return; }

    Passenger p;
    printf("\n--- Add Passenger ---\n");

    // clear leftover newline and read full name
    getchar();
    printf("Full Name: ");
    if (!fgets(p.name, sizeof(p.name), stdin)) p.name[0] = '\0';
    p.name[strcspn(p.name, "\n")] = '\0';

    printf("Age: ");
    scanf("%d", &p.age);

    printf("Ticket No: ");
    scanf("%39s", p.ticketNo);

    printf("Seat No: ");
    scanf("%11s", p.seat);

    printf("Assign Flight No (existing): ");
    scanf("%15s", p.flightNo);
    if (!getFlightByNo(p.flightNo)) {
        printf("Flight not found. Add the flight first.\n");
        return;
    }

    printf("\nSelect Category:\n");
    printf("1. VIP\n2. Elderly\n3. Business\n4. Economy\nChoice: ");
    int c; scanf("%d", &c);
    if (c == 1) p.priority = 10;
    else if (c == 2) p.priority = 8;
    else if (c == 3) p.priority = 6;
    else p.priority = 3;

    // special assist validation
    while (1) {
        printf("Special Assistance (none / wheelchair): ");
        scanf("%19s", p.assist);
        if (!strcmp(p.assist, "none") || !strcmp(p.assist, "wheelchair")) break;
        printf("Invalid input. Enter 'none' or 'wheelchair'.\n");
    }

    printf("Baggage Weight (kg): ");
    scanf("%d", &p.baggage);

    p.isCheckedIn = 0;
    p.isBoarded = 0;

    db[dbCount++] = p;
    printf("\nPassenger added to database and assigned to flight %s.\n", p.flightNo);
}
void viewPassengers() {
    if (dbCount == 0) { printf("No passengers.\n"); return; }

    printf("\n=== PASSENGER DATABASE ===\n");
    for (int i = 0; i < dbCount; ++i) {
        Passenger *p = &db[i];
        printf("Passenger #%d\n", i+1);
        printf("  Name       : %s\n", p->name);
        printf("  Age        : %d\n", p->age);
        printf("  Ticket No  : %s\n", p->ticketNo);
        printf("  Flight     : %s\n", p->flightNo);
        printf("  Seat       : %s\n", p->seat);
        printf("  Priority   : %d\n", p->priority);
        printf("  Assist     : %s\n", p->assist);
        printf("  Baggage(kg): %d\n", p->baggage);
        printf("  Checked-In : %s\n", p->isCheckedIn ? "YES" : "NO");
        printf("  Boarded    : %s\n", p->isBoarded ? "YES" : "NO");
        printf("---------------------------------------\n");
    }
}

// ---------------- Agent (Check-in & Boarding) ----------------
int findPassengerIndexByTicket(const char *ticket) {
    for (int i = 0; i < dbCount; ++i) if (!strcmp(db[i].ticketNo, ticket)) return i;
    return -1;
}

// local heap helpers for startBoarding
void localSwap(Passenger *a, Passenger *b) { Passenger t = *a; *a = *b; *b = t; }
void localHeapifyUp(Passenger heapArr[], int index) {
    if (index == 0) return;
    int parent = (index - 1) / 2;
    if (heapArr[index].priority > heapArr[parent].priority) {
        localSwap(&heapArr[index], &heapArr[parent]);
        localHeapifyUp(heapArr, parent);
    }
}
void localHeapifyDown(Passenger heapArr[], int size, int i) {
    int left = 2*i + 1, right = 2*i + 2, largest = i;
    if (left < size && heapArr[left].priority > heapArr[largest].priority) largest = left;
    if (right < size && heapArr[right].priority > heapArr[largest].priority) largest = right;
    if (largest != i) {
        localSwap(&heapArr[i], &heapArr[largest]);
        localHeapifyDown(heapArr, size, largest);
    }
}
void localInsertHeap(Passenger heapArr[], int *size, Passenger p) {
    heapArr[*size] = p;
    localHeapifyUp(heapArr, *size);
    (*size)++;
}
Passenger localExtractMax(Passenger heapArr[], int *size) {
    Passenger invalid;
    invalid.name[0] = '\0';
    if (*size == 0) return invalid;
    Passenger top = heapArr[0];
    heapArr[0] = heapArr[*size - 1];
    (*size)--;
    localHeapifyDown(heapArr, *size, 0);
    return top;
}

// Check-in: mark checked in (do not mutate global heap/queue)
void checkIn() {
    char ticket[40];
    printf("Enter Ticket No to check-in: ");
    scanf("%39s", ticket);
    int idx = findPassengerIndexByTicket(ticket);
    if (idx == -1) { printf("Passenger not found.\n"); return; }
    if (db[idx].isCheckedIn) { printf("Passenger already checked in.\n"); return; }
    db[idx].isCheckedIn = 1;
    printf("Passenger '%s' checked-in for flight %s.\n", db[idx].name, db[idx].flightNo);
}

// Start boarding for a specific flight: build local heap/queue from db[]
void startBoardingForFlight() {
    char flightNo[16];
    printf("Enter Flight No to start boarding: ");
    scanf("%15s", flightNo);
    Flight *f = getFlightByNo(flightNo);
    if (!f) { printf("Flight not found.\n"); return; }

    // build local heap & queue only for passengers of this flight who are checked-in & not boarded
    Passenger localHeap[MAX];
    int localHeapSize = 0;
    Passenger localQueue[MAX];
    int qf = -1, qr = -1;

    for (int i = 0; i < dbCount; ++i) {
        if (strcmp(db[i].flightNo, flightNo) != 0) continue;
        if (db[i].isCheckedIn && !db[i].isBoarded) {
            if (db[i].priority >= 6) localInsertHeap(localHeap, &localHeapSize, db[i]);
            else {
                if (qf == -1) qf = 0;
                localQueue[++qr] = db[i];
            }
        }
    }

    if (localHeapSize == 0 && qf == -1) {
        printf("No checked-in passengers to board for flight %s.\n", flightNo);
        return;
    }

    printf("\n=== Boarding Flight %s : %s -> %s ===\n", f->flightNo, f->source, f->destination);
    printf("Departure: %s | Status: %s\n", f->deptTime, f->status);
    printf("---------------------------------------\n");

    // board heap passengers (priority first)
    while (localHeapSize > 0) {
        Passenger p = localExtractMax(localHeap, &localHeapSize);
        int idx = findPassengerIndexByTicket(p.ticketNo);
        if (idx != -1) db[idx].isBoarded = 1;
        addBoarded(p);
        logBoardingToFile(p);
        printf("Boarded (PRIORITY)\n");
        printf("  Name   : %s\n", p.name);
        printf("  Age    : %d\n", p.age);
        printf("  Ticket : %s\n", p.ticketNo);
        printf("  Seat   : %s\n", p.seat);
        printf("---------------------------------------\n");
    }

    // board queue passengers (FIFO)
    while (qf != -1 && qf <= qr) {
        Passenger p = localQueue[qf++];
        if (qf > qr) { qf = qr = -1; } // reset
        int idx = findPassengerIndexByTicket(p.ticketNo);
        if (idx != -1) db[idx].isBoarded = 1;
        addBoarded(p);
        logBoardingToFile(p);
        printf("Boarded (NORMAL)\n");
        printf("  Name   : %s\n", p.name);
        printf("  Age    : %d\n", p.age);
        printf("  Ticket : %s\n", p.ticketNo);
        printf("  Seat   : %s\n", p.seat);
        printf("---------------------------------------\n");
    }

    printf("Boarding completed for flight %s.\n", flightNo);
}

// ---------------- Crew views ----------------
void showCheckedInWaitingForFlight(const char *flightNo) {
    int found = 0;
    printf("\n=== Checked-in & Waiting to Board for %s ===\n", flightNo);
    for (int i = 0; i < dbCount; ++i) {
        Passenger *p = &db[i];
        if (!strcmp(p->flightNo, flightNo) && p->isCheckedIn && !p->isBoarded) {
            printf("Name   : %s\n", p->name);
            printf("Age    : %d\n", p->age);
            printf("Ticket : %s\n", p->ticketNo);
            printf("Seat   : %s\n", p->seat);
            printf("Priority: %d | Assist: %s\n", p->priority, p->assist);
            printf("---------------------------------------\n");
            found = 1;
        }
    }
    if (!found) printf("No waiting passengers for flight %s.\n", flightNo);
}

// ---------------- Menus ----------------
void staffMenu() {
    int ch;
    while (1) {
        printf("\n--- STAFF MENU ---\n");
        printf("1. Add Flight\n");
        printf("2. View Flights\n");
        printf("3. Update Flight Delay/Status\n");
        printf("4. Add Passenger\n");
        printf("5. View Passengers\n");
        printf("6. Exit\n");
        printf("Choice: ");
        if (scanf("%d", &ch) != 1) { scanf("%*s"); continue; }
        if (ch == 1) addFlight();
        else if (ch == 2) viewFlights();
        else if (ch == 3) updateFlightDelay();
        else if (ch == 4) addPassenger();
        else if (ch == 5) viewPassengers();
        else if (ch == 6) break;
        else printf("Invalid choice.\n");
    }
}
void agentMenu() {
    int ch;
    while (1) {
        printf("\n--- AGENT MENU ---\n");
        printf("1. Check-In Passenger\n");
        printf("2. Start Boarding for Flight\n");
        printf("3. View Flights\n");
        printf("4. Exit\n");
        printf("Choice: ");
        if (scanf("%d", &ch) != 1) { scanf("%*s"); continue; }
        if (ch == 1) checkIn();
        else if (ch == 2) startBoardingForFlight();
        else if (ch == 3) viewFlights();
        else if (ch == 4) break;
        else printf("Invalid choice.\n");
    }
}
void crewMenu() {
    int ch;
    while (1) {
        printf("\n--- CREW MENU ---\n");
        printf("1. View Checked-In & Boarded for Flight\n");
        printf("2. View Special Assistance for Flight\n");
        printf("3. Exit\n");
        printf("Choice: ");
        if (scanf("%d", &ch) != 1) { scanf("%*s"); continue; }
        if (ch == 1) {
            char fno[16];
            printf("Enter Flight No: ");
            scanf("%15s", fno);
            showCheckedInWaitingForFlight(fno);
            showBoardedForFlight(fno);
        }
        else if (ch == 2) {
            char fno[16];
            printf("Enter Flight No: ");
            scanf("%15s", fno);
            printf("\n--- Special Assistance for %s ---\n", fno);
            int found = 0;
            for (int i = 0; i < dbCount; ++i) {
                if (!strcmp(db[i].flightNo, fno) && strcmp(db[i].assist, "none") != 0) {
                    printf("Name: %s | Assist: %s | Ticket: %s | Seat: %s\n", db[i].name, db[i].assist, db[i].ticketNo, db[i].seat);
                    found = 1;
                }
            }
            if (!found) printf("No special assistance passengers for %s.\n", fno);
        }
        else if (ch == 3) break;
        else printf("Invalid choice.\n");
    }
}

// ---------------- Main ----------------
int main() {
    char user[128], pass[128], role[64];
    int choice;

    printf("Make sure staff.txt exists with usernames and roles (e.g. staff staff123 STAFF ...)\n");

    while (1) {
        printf("\n=== Airline Boarding System ===\n");
        printf("Login as:\n");
        printf("1. Staff\n2. Gate Agent\n3. Cabin Crew\n4. Exit\n");
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) { scanf("%*s"); continue; }

        if (choice == 4) { printf("Exiting...\n"); break; }

        char expectedRole[20];
        if (choice == 1) strcpy(expectedRole, "STAFF");
        else if (choice == 2) strcpy(expectedRole, "AGENT");
        else if (choice == 3) strcpy(expectedRole, "CREW");
        else { printf("Invalid.\n"); continue; }

        printf("Username: ");
        scanf("%127s", user);
        printf("Password: ");
        scanf("%127s", pass);

        if (!authenticate(user, pass, role)) {
            printf("Invalid username or password.\n");
            continue;
        }

        if (strcmp(role, expectedRole) != 0) {
            printf("You selected %s but logged in as %s.\n", expectedRole, role);
            continue;
        }

        printf("Login successful as %s!\n", role);

        if (!strcmp(role, "STAFF")) staffMenu();
        else if (!strcmp(role, "AGENT")) agentMenu();
        else if (!strcmp(role, "CREW")) crewMenu();
    }

    return 0;
}
