import random

# Graph parameters
NUM_LOCATIONS = 50
MAX_EDGES_PER_LOC = 4

# Vehicle parameters
NUM_VEHICLES = 100
VEHICLE_TYPES = ["Ambulance", "FireTruck", "PoliceCar"]
VEHICLE_STATUS = ["Available", "Available", "EnRoute", "Maintenance"]

# Incident parameters
NUM_PENDING = 200
NUM_HISTORY = 300
INCIDENT_TYPES = ["Medical", "Fire", "Crime"]
INCIDENT_STATUS = ["Pending", "Dispatched"]

# Generate graph.dat
with open('graph.dat', 'w') as f:
    f.write(f"{NUM_LOCATIONS}\n")
    for i in range(1, NUM_LOCATIONS + 1):
        f.write(f"{i} Location_{i}\n")
    # Add edges
    for i in range(1, NUM_LOCATIONS + 1):
        num_edges = random.randint(1, MAX_EDGES_PER_LOC)
        destinations = random.sample(range(1, NUM_LOCATIONS + 1), num_edges)
        for dest in destinations:
            if dest != i:
                weight = random.randint(5, 50)
                f.write(f"E {i} {dest} {weight}\n")

# Generate vehicles.dat
with open('vehicles.dat', 'w') as f:
    for i in range(1, NUM_VEHICLES + 1):
        v_type = random.choice(VEHICLE_TYPES)
        v_loc = random.randint(1, NUM_LOCATIONS)
        v_stat = random.choice(VEHICLE_STATUS)
        f.write(f"{i} {v_type} {v_loc} {v_stat}\n")

# Generate incidents.dat
with open('incidents.dat', 'w') as f:
    f.write(f"{NUM_PENDING}\n")
    for i in range(1, NUM_PENDING + 1):
        i_loc = random.randint(1, NUM_LOCATIONS)
        i_type = random.choice(INCIDENT_TYPES)
        i_urg = random.randint(1, 5)
        i_stat = random.choice(INCIDENT_STATUS)
        f.write(f"{i} {i_loc} {i_type} {i_urg} {i_stat}\n")

# Generate history.dat
with open('history.dat', 'w') as f:
    f.write(f"{NUM_HISTORY}\n")
    for i in range(NUM_PENDING + 1, NUM_PENDING + NUM_HISTORY + 1):
        i_loc = random.randint(1, NUM_LOCATIONS)
        i_type = random.choice(INCIDENT_TYPES)
        i_urg = random.randint(1, 5)
        f.write(f"{i} {i_loc} {i_type} {i_urg} Closed Resolved_{i}\n")

# Generate tasks.dat
with open('tasks.dat', 'w') as f:
    for i in range(1, 101):
        vid = random.randint(1, NUM_VEHICLES)
        f.write(f"{vid} Maintenance_check_task_{i}\n")

print("Generated all large sample data files successfully!")
