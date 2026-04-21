// Emergency Response Dispatch System - DS Project
// All data structures built from scratch. No STL containers.

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <windows.h>
#include <conio.h>

using namespace std;

// ============================================================
// DATA TYPES
// ============================================================

struct Incident {
    int  id;
    int  locationId;
    char type[20];       // Medical Fire Crime
    int  urgency;        // 1=highest priority, 5=lowest
    char status[20];     // Pending Dispatched Closed
    char timeClosed[20];
};

struct Vehicle {
    int  id;
    char type[20];       // Ambulance FireTruck PoliceCar
    int  locationId;
    char status[20];     // Available EnRoute Maintenance
    Vehicle* next;
};

struct Task {
    int  vehicleId;
    char desc[60];
    Task* next;
};

struct Action {
    char desc[80];
    Action* next;
};

struct Edge {
    int  dest, weight;
    Edge* next;
};

struct Vertex {
    int  id;
    char name[30];
    Edge* edges;
};

struct AVLNode {
    Incident data;
    int height;
    AVLNode* left;
    AVLNode* right;
};

// ============================================================
// MIN-HEAP (incident priority queue)
// ============================================================

struct MinHeap {
    Incident arr[300];
    int size;

    MinHeap() { size = 0; }

    void swp(int a, int b) {
        Incident t = arr[a]; arr[a] = arr[b]; arr[b] = t;
    }

    // heapify up
    void heapUp(int i) {
        while (i > 0) {
            int p = (i-1)/2;
            if (arr[p].urgency > arr[i].urgency) { swp(p, i); i = p; }
            else break;
        }
    }

    // heapify down
    void heapDown(int i) {
        while (true) {
            int l=2*i+1, r=2*i+2, m=i;
            if (l<size && arr[l].urgency < arr[m].urgency) m=l;
            if (r<size && arr[r].urgency < arr[m].urgency) m=r;
            if (m==i) break;
            swp(m, i); i=m;
        }
    }

    bool insert(Incident inc) {
        if (size >= 300) return false;
        arr[size++] = inc;
        heapUp(size-1);
        return true;
    }

    bool extractMin(Incident &out) {
        if (size == 0) return false;
        out = arr[0];
        arr[0] = arr[--size];
        heapDown(0);
        return true;
    }

    bool exists(int id) {
        for (int i = 0; i < size; i++)
            if (arr[i].id == id) return true;
        return false;
    }

    void display() {
        for (int i = 0; i < size; i++) {
            cout << "  INC-" << arr[i].id
                 << " | " << arr[i].type
                 << " | Urgency:" << arr[i].urgency
                 << " | Loc:" << arr[i].locationId
                 << " | " << arr[i].status << "\n";
        }
    }
};

// ============================================================
// HASH TABLE (vehicle lookup, chaining)
// ============================================================

const int HASH_SZ = 53;

struct HashTable {
    Vehicle* buckets[HASH_SZ];

    HashTable() { for (int i=0;i<HASH_SZ;i++) buckets[i]=nullptr; }

    int hash(int id) { return id % HASH_SZ; }

    void insert(Vehicle v) {
        int h = hash(v.id);
        Vehicle* node = new Vehicle;
        *node = v; node->next = buckets[h];
        buckets[h] = node;
    }

    Vehicle* find(int id) {
        Vehicle* cur = buckets[hash(id)];
        while (cur) { if (cur->id==id) return cur; cur=cur->next; }
        return nullptr;
    }

    bool remove(int id) {
        int h = hash(id);
        Vehicle* cur = buckets[h]; Vehicle* prev = nullptr;
        while (cur) {
            if (cur->id == id) {
                if (prev) prev->next = cur->next; else buckets[h] = cur->next;
                delete cur; return true;
            }
            prev=cur; cur=cur->next;
        }
        return false;
    }

    void display() {
        bool any = false;
        for (int i=0;i<HASH_SZ;i++) {
            Vehicle* cur = buckets[i];
            while (cur) {
                cout << "  V-" << cur->id << " | " << cur->type
                     << " | Loc:" << cur->locationId
                     << " | " << cur->status << "\n";
                cur=cur->next; any=true;
            }
        }
        if (!any) cout << "  (no vehicles)\n";
    }
};

// ============================================================
// GRAPH + BFS (city map, proximity search)
// ============================================================

const int MAX_V = 60;

struct Graph {
    Vertex verts[MAX_V];
    int count;

    Graph() { count=0; }

    int idx(int id) {
        for (int i=0;i<count;i++) if (verts[i].id==id) return i;
        return -1;
    }

    bool addVertex(int id, const char* name) {
        if (count>=MAX_V || idx(id)!=-1) return false;
        verts[count].id=id;
        strncpy(verts[count].name,name,29);
        verts[count].edges=nullptr;
        count++;
        return true;
    }

    bool addEdge(int from, int to, int w) {
        int fi = idx(from); if (fi==-1) return false;
        Edge* e = new Edge; e->dest=to; e->weight=w;
        e->next = verts[fi].edges; verts[fi].edges=e;
        return true;
    }

    void removeVertex(int id) {
        int fi = idx(id); if (fi==-1) return;
        Edge* e = verts[fi].edges;
        while (e) { Edge* t=e->next; delete e; e=t; }
        for (int i=fi;i<count-1;i++) verts[i]=verts[i+1];
        count--;
    }

    // BFS to find closest available vehicle of given type
    int bfsClosest(int startId, HashTable &ht, const char* vtype) {
        bool visited[MAX_V] = {};
        int  queue[MAX_V];
        int  front=0, back=0;
        int  si = idx(startId);
        if (si==-1) return -1;
        queue[back++]=si; visited[si]=true;
        while (front<back) {
            int ci = queue[front++];
            // check vehicles at this location
            for (int h=0;h<HASH_SZ;h++) {
                Vehicle* v = ht.buckets[h];
                while (v) {
                    if (v->locationId==verts[ci].id &&
                        strcmp(v->status,"Available")==0 &&
                        strcmp(v->type, vtype)==0)
                        return v->id;
                    v=v->next;
                }
            }
            Edge* e = verts[ci].edges;
            while (e) {
                int ni=idx(e->dest);
                if (ni!=-1 && !visited[ni]) { visited[ni]=true; queue[back++]=ni; }
                e=e->next;
            }
        }
        return -1;
    }

    void display() {
        for (int i=0;i<count;i++) {
            cout << "  Loc " << verts[i].id << " - " << verts[i].name << " -> ";
            Edge* e=verts[i].edges;
            if (!e) cout << "(no roads)";
            while (e) { cout << verts[e->dest>0?idx(e->dest):0].id << "(w:"<<e->weight<<") "; e=e->next; }
            cout << "\n";
        }
    }
};

// ============================================================
// AVL TREE (historical log, sorted by incident ID)
// ============================================================

int avlH(AVLNode* n) { return n ? n->height : 0; }
int avlB(AVLNode* n) { return n ? avlH(n->left)-avlH(n->right) : 0; }
void avlFix(AVLNode* n) { if (n) n->height=1+max(avlH(n->left),avlH(n->right)); }

AVLNode* rotR(AVLNode* y) {
    AVLNode* x=y->left; y->left=x->right; x->right=y;
    avlFix(y); avlFix(x); return x;
}
AVLNode* rotL(AVLNode* x) {
    AVLNode* y=x->right; x->right=y->left; y->left=x;
    avlFix(x); avlFix(y); return y;
}

AVLNode* avlInsert(AVLNode* n, Incident inc) {
    if (!n) {
        AVLNode* nd=new AVLNode; nd->data=inc;
        nd->height=1; nd->left=nd->right=nullptr; return nd;
    }
    if (inc.id < n->data.id)      n->left =avlInsert(n->left,inc);
    else if (inc.id > n->data.id) n->right=avlInsert(n->right,inc);
    else return n;
    avlFix(n);
    int b=avlB(n);
    // LL
    if (b>1  && inc.id < n->left->data.id)  return rotR(n);
    // RR
    if (b<-1 && inc.id > n->right->data.id) return rotL(n);
    // LR
    if (b>1  && inc.id > n->left->data.id)  { n->left=rotL(n->left); return rotR(n); }
    // RL
    if (b<-1 && inc.id < n->right->data.id) { n->right=rotR(n->right); return rotL(n); }
    return n;
}

void avlInOrder(AVLNode* n) {
    if (!n) return;
    avlInOrder(n->left);
    cout << "  INC-" << n->data.id
         << " | " << n->data.type
         << " | Urgency:" << n->data.urgency
         << " | " << n->data.status
         << " | Closed:" << n->data.timeClosed << "\n";
    avlInOrder(n->right);
}

AVLNode* avlSearch(AVLNode* n, int id) {
    if (!n) return nullptr;
    if (id==n->data.id) return n;
    return id<n->data.id ? avlSearch(n->left,id) : avlSearch(n->right,id);
}

// ============================================================
// QUEUE (maintenance tasks, FIFO with linked list)
// ============================================================

struct Queue {
    Task* front;
    Task* rear;

    Queue() { front=rear=nullptr; }

    void enqueue(int vid, const char* d) {
        Task* t=new Task; t->vehicleId=vid;
        strncpy(t->desc,d,59); t->next=nullptr;
        if (!rear) { front=rear=t; return; }
        rear->next=t; rear=t;
    }

    bool dequeue(Task &out) {
        if (!front) return false;
        out=*front; Task* tmp=front;
        front=front->next;
        if (!front) rear=nullptr;
        delete tmp; return true;
    }

    void display() {
        Task* cur=front;
        if (!cur) { cout<<"  (empty)\n"; return; }
        while (cur) { cout<<"  V-"<<cur->vehicleId<<": "<<cur->desc<<"\n"; cur=cur->next; }
    }
};

// ============================================================
// STACK (command history, LIFO with linked list)
// ============================================================

struct Stack {
    Action* top;
    Stack() { top=nullptr; }

    void push(const char* d) {
        Action* a=new Action; strncpy(a->desc,d,79);
        a->next=top; top=a;
    }

    bool pop(Action &out) {
        if (!top) return false;
        out=*top; Action* tmp=top;
        top=top->next; delete tmp; return true;
    }

    void peek() {
        if (top) cout<<"  Last: "<<top->desc<<"\n";
        else cout<<"  (empty)\n";
    }

    void display() {
        Action* cur=top;
        if (!cur) { cout<<"  (empty)\n"; return; }
        while (cur) { cout<<"  "<<cur->desc<<"\n"; cur=cur->next; }
    }
};

// ============================================================
// GLOBALS
// ============================================================

MinHeap   heap;
HashTable ht;
Graph     graph;
AVLNode*  avlRoot = nullptr;
Queue     taskQ;
Stack     cmdStack;

// ============================================================
// CONSOLE HELPERS
// ============================================================

void color(int c) { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c); }

void cls() { system("cls"); }

void pause() { cout << "\n  Press any key..."; _getch(); }

void header(const char* t) {
    color(11); cout << "\n  ===== " << t << " =====\n"; color(7);
}

// ============================================================
// FILE I/O
// ============================================================

void loadAll() {
    // load incidents
    ifstream fi("incidents.dat");
    if (fi) {
        int n; fi >> n;
        for (int i=0;i<n;i++) {
            Incident inc;
            fi >> inc.id >> inc.locationId >> inc.type >> inc.urgency >> inc.status;
            strcpy(inc.timeClosed, "N/A");
            if (!heap.exists(inc.id)) heap.insert(inc);
        }
        fi.close();
    }

    // load vehicles
    ifstream fv("vehicles.dat");
    if (fv) {
        Vehicle v; v.next=nullptr;
        while (fv >> v.id >> v.type >> v.locationId >> v.status) {
            if (!ht.find(v.id)) { v.next=nullptr; ht.insert(v); }
        }
        fv.close();
    }

    // load graph
    ifstream fg("graph.dat");
    if (fg) {
        int cnt; fg >> cnt;
        for (int i=0;i<cnt;i++) {
            int id; char name[30];
            fg >> id >> name;
            graph.addVertex(id, name);
        }
        char tag; int f,t,w;
        while (fg >> tag >> f >> t >> w)
            graph.addEdge(f,t,w);
        fg.close();
    }
}

void saveAll() {
    // save heap (incidents)
    ofstream fi("incidents.dat");
    fi << heap.size << "\n";
    for (int i=0;i<heap.size;i++) {
        Incident& inc=heap.arr[i];
        fi << inc.id << " " << inc.locationId << " " << inc.type
           << " " << inc.urgency << " " << inc.status << "\n";
    }
    fi.close();

    // save vehicles
    ofstream fv("vehicles.dat");
    for (int h=0;h<HASH_SZ;h++) {
        Vehicle* cur=ht.buckets[h];
        while (cur) {
            fv << cur->id << " " << cur->type << " "
               << cur->locationId << " " << cur->status << "\n";
            cur=cur->next;
        }
    }
    fv.close();

    // save graph
    ofstream fg("graph.dat");
    fg << graph.count << "\n";
    for (int i=0;i<graph.count;i++) {
        fg << graph.verts[i].id << " " << graph.verts[i].name << "\n";
        Edge* e=graph.verts[i].edges;
        while (e) { fg << "E " << graph.verts[i].id << " " << e->dest << " " << e->weight << "\n"; e=e->next; }
    }
    fg.close();

    cout << "\n  State saved.\n";
}

// ============================================================
// ARROW-KEY MENU
// ============================================================

int arrowMenu(const char* items[], int count, const char* title) {
    int sel=0;
    while (true) {
        cls();
        color(11); cout << "\n  " << title << "\n\n";
        for (int i=0;i<count;i++) {
            if (i==sel) { color(14); cout << "  >> " << items[i] << "\n"; }
            else         { color(7);  cout << "     " << items[i] << "\n"; }
        }
        color(8); cout << "\n  Arrow keys or W/S = Navigate | Enter = Select\n"; color(7);
        int k=_getch();
        if (k==0||k==224) {
            k=_getch();
            if (k==72) sel=(sel-1+count)%count;
            if (k==80) sel=(sel+1)%count;
        } else if (k=='w'||k=='W') sel=(sel-1+count)%count;
          else if (k=='s'||k=='S') sel=(sel+1)%count;
          else if (k==13) return sel;
          else if (k==27) return count-1;
    }
}

// ============================================================
// OPERATIONS
// ============================================================

void logIncident() {
    header("Log New Incident");
    Incident inc;
    cout << "  Incident ID: "; cin >> inc.id;
    if (heap.exists(inc.id)) { cout << "  Incident ID already exists.\n"; return; }
    cout << "  Location ID: "; cin >> inc.locationId;
    cout << "  Type (Medical/Fire/Crime): "; cin >> inc.type;
    cout << "  Urgency 1=High 5=Low: "; cin >> inc.urgency;
    strcpy(inc.status, "Pending");
    strcpy(inc.timeClosed, "N/A");
    if (heap.insert(inc)) {
        char msg[80]; sprintf(msg,"Logged INC-%d (%s) Urgency:%d",inc.id,inc.type,inc.urgency);
        cmdStack.push(msg);
        cout << "  Incident logged.\n";
    } else cout << "  Heap full.\n";
}

void dispatchNext() {
    header("Dispatch Next Incident");
    Incident inc;
    if (!heap.extractMin(inc)) { cout << "  Heap is empty. No pending incidents.\n"; return; }
    cout << "  Top incident: INC-" << inc.id << " (" << inc.type << ") at Loc-" << inc.locationId << "\n";

    char vtype[20];
    if (strcmp(inc.type,"Medical")==0) strcpy(vtype,"Ambulance");
    else if (strcmp(inc.type,"Fire")==0) strcpy(vtype,"FireTruck");
    else strcpy(vtype,"PoliceCar");

    int vid = graph.bfsClosest(inc.locationId, ht, vtype);
    if (vid==-1) {
        cout << "  No available vehicle of type " << vtype << " found within range.\n";
        heap.insert(inc); // put back
        return;
    }
    Vehicle* v = ht.find(vid);
    strcpy(v->status,"EnRoute");
    v->locationId = inc.locationId;
    strcpy(inc.status,"Dispatched");

    char msg[80]; sprintf(msg,"Dispatched V-%d to INC-%d",vid,inc.id);
    cmdStack.push(msg);
    cout << "  " << msg << "\n";
}

void resolveIncident() {
    header("Resolve/Close Incident");
    int id, vid;
    cout << "  Incident ID to close: "; cin >> id;
    cout << "  Vehicle ID to free: ";   cin >> vid;
    Vehicle* v = ht.find(vid);
    if (!v) { cout << "  Vehicle V-" << vid << " not found.\n"; return; }
    strcpy(v->status,"Available");

    Incident inc; inc.id=id; inc.urgency=0;
    strcpy(inc.type,"Closed"); strcpy(inc.status,"Closed");
    strcpy(inc.timeClosed,"Resolved"); inc.locationId=0;
    avlRoot = avlInsert(avlRoot, inc);

    char msg[80]; sprintf(msg,"Closed INC-%d, freed V-%d",id,vid);
    cmdStack.push(msg);
    cout << "  " << msg << "\n";
}

void viewIncidents() { header("Pending Incidents (Min-Heap)"); heap.display(); }

void addVehicle() {
    header("Add Vehicle");
    Vehicle v; v.next=nullptr;
    cout << "  Vehicle ID: "; cin >> v.id;
    if (ht.find(v.id)) { cout << "  Vehicle already exists.\n"; return; }
    cout << "  Type (Ambulance/FireTruck/PoliceCar): "; cin >> v.type;
    cout << "  Location ID: "; cin >> v.locationId;
    strcpy(v.status,"Available");
    ht.insert(v);
    cout << "  Vehicle V-" << v.id << " added.\n";
}

void removeVehicle() {
    header("Remove Vehicle");
    int id; cout << "  Vehicle ID: "; cin >> id;
    if (ht.remove(id)) cout << "  Removed.\n";
    else cout << "  Vehicle V-" << id << " not found.\n";
}

void updateVehicle() {
    header("Update Vehicle");
    int id; cout << "  Vehicle ID: "; cin >> id;
    Vehicle* v=ht.find(id);
    if (!v) { cout << "  Vehicle V-" << id << " not found.\n"; return; }
    cout << "  New Status (Available/EnRoute/Maintenance): "; cin >> v->status;
    cout << "  New Location ID: "; cin >> v->locationId;
    cout << "  Updated.\n";
}

void addLocation() {
    header("Add Map Location");
    int id; char name[30];
    cout << "  Location ID: "; cin >> id;
    cout << "  Name: "; cin >> name;
    if (graph.addVertex(id,name)) cout << "  Location added.\n";
    else cout << "  ID already exists or limit reached.\n";
}

void addRoad() {
    header("Add Road");
    int f,t,w;
    cout << "  From Location ID: "; cin >> f;
    cout << "  To Location ID: ";   cin >> t;
    cout << "  Weight (distance): "; cin >> w;
    if (graph.addEdge(f,t,w)) cout << "  Road added.\n";
    else cout << "  Source location not found.\n";
}

void addTask() {
    header("Add Maintenance Task");
    int vid; char d[60];
    cout << "  Vehicle ID: "; cin >> vid;
    cout << "  Description: "; cin.ignore(); cin.getline(d,59);
    taskQ.enqueue(vid,d);
    cout << "  Task queued.\n";
}

void processTask() {
    header("Process Next Task");
    Task t;
    if (taskQ.dequeue(t)) cout << "  Done: V-" << t.vehicleId << " - " << t.desc << "\n";
    else cout << "  No tasks pending.\n";
}

void popHistory() {
    header("Pop Last Action");
    Action a;
    if (cmdStack.pop(a)) cout << "  Popped: " << a.desc << "\n";
    else cout << "  Stack is empty.\n";
}

void searchHistorical() {
    header("Search Historical Record");
    int id; cout << "  Incident ID: "; cin >> id;
    AVLNode* n=avlSearch(avlRoot,id);
    if (n) cout << "  Found: INC-" << n->data.id << " | " << n->data.status << " | Closed:" << n->data.timeClosed << "\n";
    else cout << "  INC-" << id << " not found in historical log.\n";
}

// ============================================================
// BANNER
// ============================================================

void showBanner() {
    cls(); color(12);
    cout << "\n\n";
    cout << "  +--------------------------------------------------+\n";
    color(11);
    cout << "  |  Emergency Response Dispatch System (ERDS Lite)  |\n";
    color(14);
    cout << "  |     Data Structures Project - C++ Console        |\n";
    color(12);
    cout << "  +--------------------------------------------------+\n";
    color(8);
    cout << "\n  Loading data from files...\n";
    color(7);
    Sleep(800);
}

// ============================================================
// MAIN
// ============================================================

int main() {
    showBanner();
    loadAll();

    color(10); cout << "\n  Data loaded successfully.\n"; color(7);
    cout << "  Incidents in queue: " << heap.size << "\n";
    Sleep(1000);

    const char* mainItems[] = {
        "Incident Dispatch",
        "Vehicle Management",
        "Map Management",
        "Administration & Reports",
        "Save State",
        "Exit"
    };

    while (true) {
        int c = arrowMenu(mainItems, 6, "ERDS -- Main Menu");

        if (c == 0) {
            const char* items[] = {"Log New Incident","View All Pending","Dispatch Next","Resolve Incident","Back"};
            int r = arrowMenu(items, 5, "Incident Dispatch");
            if (r==0) logIncident();
            else if (r==1) { header("All Pending Incidents"); viewIncidents(); }
            else if (r==2) dispatchNext();
            else if (r==3) resolveIncident();
        } else if (c == 1) {
            const char* items[] = {"Add Vehicle","Remove Vehicle","Update Vehicle","List All Vehicles","Back"};
            int r = arrowMenu(items, 5, "Vehicle Management");
            if (r==0) addVehicle();
            else if (r==1) removeVehicle();
            else if (r==2) updateVehicle();
            else if (r==3) { header("All Vehicles"); ht.display(); }
        } else if (c == 2) {
            const char* items[] = {"Add Location","Add Road","View City Map","Back"};
            int r = arrowMenu(items, 4, "Map Management");
            if (r==0) addLocation();
            else if (r==1) addRoad();
            else if (r==2) { header("City Map"); graph.display(); }
        } else if (c == 3) {
            const char* items[] = {"Add Maintenance Task","View Task Queue","Process Next Task",
                                   "View Command History","Pop Last Action",
                                   "Historical Report (AVL)","Search Historical","Back"};
            int r = arrowMenu(items, 8, "Administration & Reports");
            if (r==0) addTask();
            else if (r==1) { header("Maintenance Queue"); taskQ.display(); }
            else if (r==2) processTask();
            else if (r==3) { header("Command History (Stack)"); cmdStack.display(); }
            else if (r==4) popHistory();
            else if (r==5) { header("Historical Incidents (AVL In-Order)"); avlInOrder(avlRoot); }
            else if (r==6) searchHistorical();
        } else if (c == 4) {
            saveAll();
        } else {
            saveAll(); break;
        }

        pause();
    }

    color(11); cout << "\n\n  ERDS shut down. Goodbye.\n"; color(7);
    return 0;
}
