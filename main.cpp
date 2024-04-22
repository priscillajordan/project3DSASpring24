#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <cstdlib>
#include <cmath>
#include <queue>
#include <unordered_set>
#include <limits>
#include <chrono>

using namespace std;

struct Hospital
{
    string name;
    string city;
    string state;
    int rating_overall;
    float longitude;
    float latitude;

    Hospital(){};
    Hospital(string name, string city, string state, int rating) : name(name), city(city), state(state), rating_overall(rating){};

    Hospital(const Hospital &other)
        : name(other.name),
          city(other.city),
          state(other.state),
          rating_overall(other.rating_overall),
          longitude(other.longitude),
          latitude(other.latitude){};

    Hospital(const string name0, int longitude0, int latitude0)
        : name(name0),
          latitude(latitude0),
          longitude(longitude0){};

    void setName(const string &name)
    {
        this->name = name;
    }
    void setCity(const string &city) { this->city = city; }
    void setLongitude(int longitude) { this->longitude = longitude; }
    void setLatitude(int latitude) { this->latitude = latitude; }

    const string &getName() const { return name; }
    const string &getCity() const { return city; }
    int getLongitude() const { return longitude; }
    int getLatitude() const { return latitude; }

    ~Hospital() {}
};

struct Data
{
    int hospital_name_index = 0;
    int city_index = 1;
    int state_index = 2;
    int rating_index = 4;

    vector<Hospital> parseData(string filename)
    {
        ifstream file(filename);
        if (!file.is_open())
        {
            cout << "Error: File not opened." << endl;
            throw("error");
        }

        // Create a vector of Hospital objects to store the data
        vector<Hospital> hospitals;

        // Read the data from the file
        string line;

        // skips first line in csv file
        getline(file, line);

        while (getline(file, line))
        {
            line.erase(remove(line.begin(), line.end(), '"'), line.end());

            // Split the line into tokens
            vector<string> tokens;
            istringstream iss(line);
            string token;
            while (getline(iss, token, ','))
            {

                tokens.push_back(token);
            }

            /*csv structure:
            index:
                0 - hospital name
                1 - city
                2- state
                4- rating (overall)
            */

            // Create a Hospital object from the tokens and add it to the vector
            if (tokens.size() >= 4)
            {
                Hospital hospital(tokens[hospital_name_index], tokens[city_index], tokens[state_index], stoi(tokens[rating_index]));
                hospitals.push_back(hospital);
            }
        }
        file.close();
        return hospitals;
    }
};

vector<Hospital> filter_by_state(string state, vector<Hospital> &hospitals)
{
    vector<Hospital> filtered;

    for (int i = 0; i < hospitals.size(); i++)
    {
        if (hospitals[i].state == state)
        {
            filtered.push_back(hospitals[i]);
        }
    }
    return filtered;
}

Hospital get_starting_hospital(vector<Hospital> &filtered)
{
    Hospital source;

    string starting_hospital_string;
    std::cout << "Enter the name of the hospital you are in currently: " << endl;
    std::cin >> std::ws; // Clear whitespace characters from the input buffer - taken from https://stackoverflow.com/questions/68341599/stdws-vs-stdskipws-in-c
    std::getline(std::cin, starting_hospital_string);

    for (Hospital &hospital : filtered)
    {
        if (hospital.name == starting_hospital_string)
        {
            source = hospital;
            break;
        }
    }
    return source;
}
Hospital get_destination_hospital(vector<Hospital> &filtered)
{
    Hospital dest;

    string dest_hospital_string;
    cout << "Enter the name of the hospital you would like to go to: " << endl;
    cin >> std::ws; // Clear whitespace characters from the input buffer
    getline(cin, dest_hospital_string);

    for (Hospital &hospital : filtered)
    {
        if (hospital.name == dest_hospital_string)
        {
            dest = hospital;
            break;
        }
    }
    return dest;
}

double toRadians(double degrees)

{
    double pi = 3.14159265358979323846;
    return degrees * (pi / 180.0);
}

float calculate_distance(float lat1, float long1, float lat2, float long2) // calculates distance between coords using Haversine formula - adapted from https://stackoverflow.com/questions/1420045/how-to-find-distance-from-the-latitude-and-longitude-of-two-locations/1422562#1422562
{
    double R = 6371.0;
    double dLat = toRadians(lat2 - lat1);
    double dLon = toRadians(long2 - long1);
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(toRadians(lat1)) * cos(toRadians(lat2)) *
                   sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    double distance = R * c;
    return distance;
}

unordered_map<string, unordered_map<string, float>> initialize_graph(vector<Hospital> &filtered, string &source_name, string &dest_name)
{
    unordered_map<string, unordered_map<string, float>> distance_map;

    for (Hospital source_hospital : filtered)
    {
        unordered_map<string, float> from_distances;
        for (Hospital destination_hospital : filtered)
        {
            if (source_hospital.name == destination_hospital.name)
            {
                continue; // Skip adding the edge to itself
            }
            else if (source_hospital.name == source_name && destination_hospital.name == dest_name)
            {
                continue; // Skip adding the direct edge from source to destination (artificially create path between hospitals lol)
            }
            else
            {
                float distance = calculate_distance(source_hospital.latitude, source_hospital.longitude, destination_hospital.latitude, destination_hospital.longitude);
                from_distances.emplace(destination_hospital.name, distance);
            }
        }
        distance_map.emplace(source_hospital.name, from_distances);
    }

    return distance_map;
}

unordered_map<string, Hospital> dijkstra(const unordered_map<string, unordered_map<string, float>> &distances, const string &start, const string &end)
{
    unordered_map<string, float> shortest_distances;
    unordered_map<string, string> predecessors; // Map to store predecessors in the shortest path
    priority_queue<pair<float, string>, vector<pair<float, string>>, greater<pair<float, string>>> pq;
    vector<string> traversal_order;

    // Initialize distances and predecessors
    for (const auto &[hospital, _] : distances) // accesses hospitals, ignores map which is the second element in the pair
    {
        shortest_distances[hospital] = numeric_limits<float>::infinity();
        predecessors[hospital] = "";
    }

    pq.push({0, start});
    shortest_distances[start] = 0;

    while (!pq.empty())
    {
        auto top_pair = pq.top(); // Get the top pair from the priority queue
        pq.pop();                 // Remove the top pair from the priority queue

        float curr_distance = top_pair.first;   // Extract the distance from the pair
        string curr_hospital = top_pair.second; // Extract the hospital name from the pair

        if (curr_hospital == end) // Terminate when reaching the destination
        {
            break;
        }

        traversal_order.push_back(curr_hospital); // Add the current hospital to traversal order

        for (const auto &[neighbor, edge_weight] : distances.at(curr_hospital))
        {
            float new_distance = curr_distance + edge_weight;
            if (new_distance < shortest_distances[neighbor])
            {
                shortest_distances[neighbor] = new_distance;
                pq.push({new_distance, neighbor});
                predecessors[neighbor] = curr_hospital; // Update predecessor for the shortest path
            }
        }
    }

    // Reconstruct the shortest path
    unordered_map<string, Hospital> shortest_path;
    string current_hospital = end;
    while (!current_hospital.empty())
    {
        shortest_path[current_hospital] = Hospital(current_hospital, 0, 0); // Create Hospital object with dummy coordinates
        current_hospital = predecessors[current_hospital];
    }

    // Print traversal order
    cout << "The quickest path to visit " << end << " from " << start << " with intermediate hospital visits is shown below: " << endl;

    for (int i = 0; i < traversal_order.size(); i++)
    {
        cout << i << ". " << traversal_order[i];
        if (i < traversal_order.size() - 1)
        {
            cout << endl;
        }
    }
    cout << endl;

    return shortest_path;
}

int main()
{
    Data data;
    vector<Hospital> hospitals = data.parseData("hospitals.csv");

    string user_state;
    cout << endl << endl << "Enter a state abbreviation. (Ex: Texas would be entered as TX)" << endl;
    cin >> user_state;

    vector<Hospital> filtered = filter_by_state(user_state, hospitals);
    cout << endl
         << endl
         << "There are " << filtered.size() << " hospitals in your state" << endl
         << endl;

    Hospital source = get_starting_hospital(filtered);
    cout << endl;

    Hospital destination = get_destination_hospital(filtered);
    cout << endl;

    cout << endl
         << endl
         << "Please wait while we gather hopsital data..." << endl
         << endl;

    // Write hospital name, city, and state to a txt file
    ofstream source_hospital_data("source_hospital_data.txt");
    if (!source_hospital_data.is_open())
    {
        cout << "Error: Unable to open file for writing." << endl;
        return 1;
    }
    source_hospital_data << source.name << endl
                         << source.city << endl
                         << source.state << endl;
    source_hospital_data.close();

    // executes python script to retrieve longitude/longitude using geocoding
    system("python geocoding.py");
    ifstream lat_long("lat_long.txt");
    if (!lat_long.is_open())
    {
        cout << "Error: Unable to open long_lat file" << endl;
        return 1;
    }

    // sets source hospital coordinates
    string source_longitude, source_latitude;
    if (getline(lat_long, source_latitude) && getline(lat_long, source_longitude))
    {
        source.latitude = stof(source_latitude);
        source.longitude = stof(source_longitude);
    }

    ofstream coords("vector_hospital_names.txt", ios::app);

    // set coords for all filtered hospitals
    for (int i = 0; i < filtered.size(); i++)
    {
        coords.open("vector_hospital_names.txt", ios::out | ios::trunc);
        coords << filtered[i].name << endl
               << filtered[i].city << endl
               << filtered[i].state << endl;
        coords.close();

        system("python geocoding_vector.py");
        ifstream lat_long_vector("hospitals_names.txt");
        string filtered_long, filtered_lat;
        if (getline(lat_long_vector, filtered_lat) && getline(lat_long_vector, filtered_long))
        {
            filtered[i].latitude = stof(filtered_lat);
            filtered[i].longitude = stof(filtered_long);
        }
        lat_long_vector.close();
    }

    cout << endl
         << "Please wait while we intialize the graph of hospitals..." << endl
         << endl;
    unordered_map<string, unordered_map<string, float>> distances = initialize_graph(filtered, source.name, destination.name);

    cout << endl
         << "Executing Dijkstra's algorithm..." << endl
         << endl;

    // timer for dijsktras runtime
    auto dijkstras_timer_start = std::chrono::high_resolution_clock::now();
    unordered_map<string, Hospital> shortest_path = dijkstra(distances, source.name, destination.name);
    auto dijkstras_timer_stop = std::chrono::high_resolution_clock::now();
    auto dijkstras_duration = std::chrono::duration_cast<std::chrono::microseconds>(dijkstras_timer_stop - dijkstras_timer_start);

    cout << endl
         << endl
         << "Dijsktra's algorithm has taken " << dijkstras_duration.count() << " microseconds to find the shortest path from "
         << source.name << " to " << destination.name << endl << endl;
}