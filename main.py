import csv
import math

class Hospital:
    def __init__(self, name, city, state, rating_overall, longitude=None, latitude=None):
        self.name = name
        self.city = city
        self.state = state
        self.rating_overall = rating_overall
        self.longitude = longitude
        self.latitude = latitude

def parse_data(filename):
    hospitals = []
    with open(filename, newline='') as csvfile:
        reader = csv.reader(csvfile)
        next(reader)  # Skip header row
        for row in reader:
            name = row[0]
            city = row[1]
            state = row[2]
            rating = int(row[4])
            hospital = Hospital(name, city, state, rating)
            hospitals.append(hospital)
    return hospitals

def filter_by_state(state, hospitals):
    filtered = [hospital for hospital in hospitals if hospital.state == state]
    return filtered

def calculate_distance(lat1, long1, lat2, long2):
    R = 6371.0
    dLat = math.radians(lat2 - lat1)
    dLon = math.radians(long2 - long1)
    a = math.sin(dLat / 2) * math.sin(dLat / 2) + math.cos(math.radians(lat1)) * math.cos(math.radians(lat2)) * math.sin(dLon / 2) * math.sin(dLon / 2)
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))
    distance = R * c
    return distance

def initialize_graph(hospitals):
    distance_map = {}
    for source_hospital in hospitals:
        from_distances = {}
        for destination_hospital in hospitals:
            if source_hospital.name == destination_hospital.name:
                continue
            distance = calculate_distance(source_hospital.latitude, source_hospital.longitude, destination_hospital.latitude, destination_hospital.longitude)
            from_distances[destination_hospital.name] = distance
        distance_map[source_hospital.name] = from_distances
    return distance_map

def main():
    hospitals = parse_data('hospitals.csv')

    user_state = input("Enter a state abbreviation (Ex: Texas would be entered as TX): ")
    filtered = filter_by_state(user_state, hospitals)
    print(f"There are {len(filtered)} hospitals in your state")

    source_hospital_name = input("Enter the name of the hospital you are currently in: ")
    source = next((hospital for hospital in filtered if hospital.name == source_hospital_name), None)
    if source is None:
        print("Error: Hospital not found.")
        return

    dest_hospital_name = input("Enter the name of the hospital you would like to go to: ")
    destination = next((hospital for hospital in filtered if hospital.name == dest_hospital_name), None)
    if destination is None:
        print("Error: Destination hospital not found.")
        return

    # Initialize graph
    distances = initialize_graph(filtered)

    print(f"The distance between {source.name} and {destination.name} is: {distances[source.name][destination.name]} kilometers.")

if __name__ == "__main__":
    main()
