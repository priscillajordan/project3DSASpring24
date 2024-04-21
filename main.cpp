#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <cstdlib>
#include <cmath>

using namespace std;

struct Hospital
{
    string name;
    string city;
    string state;
    int rating_overall;
    float longitude;
    float latitude;
    vector<Hospital> neighbors;

    Hospital(){};
    Hospital(string name, string city, string state, int rating) : name(name), city(city), state(state), rating_overall(rating){};

    void setName(const string &name) { this->name = name; }
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

double toRadians(double degrees)
{
    return degrees * (3.14159 / 180.0);
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

unordered_map<string, unordered_map<string, float>> initialize_graph(vector<Hospital> &filtered)
{ // creates graph using filtered hospitals vector
    unordered_map<string, unordered_map<string, float>> distance_map;
    for (Hospital source_hospital : filtered)
    {
        unordered_map<string, float> from_distances;
        for (Hospital destination_hospital : filtered)
        {
            if (source_hospital.name == destination_hospital.name)
            {
                continue;
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

int main()
{
    Data data;
    vector<Hospital> hospitals = data.parseData("hospitals.csv");

    string user_state;
    std::cout << "Enter a state abbreviation. (Ex: Texas would be entered as TX)" << endl;
    std::cin >> user_state;

    vector<Hospital> filtered = filter_by_state(user_state, hospitals);
    std::cout << "There are " << filtered.size() << " hospitals in your state" << endl;

    Hospital source = get_starting_hospital(filtered);
    cout << endl;

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
}
