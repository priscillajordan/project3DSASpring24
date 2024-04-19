#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>


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

Hospital get_starting_hospital(vector<Hospital> filtered) {
    Hospital source;

    string starting_hospital_string;
    std::cout << "Enter the name of the hospital you are in currently: " << endl;
    std::cin >> std::ws;  // Clear whitespace characters from the input buffer - taken from https://stackoverflow.com/questions/68341599/stdws-vs-stdskipws-in-c
    std::getline(std::cin, starting_hospital_string);

    for (Hospital& hospital : filtered) {
        if (hospital.name == starting_hospital_string) {
            source = hospital;
            break;
        }
    } 
    return source;
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
    source_hospital_data << source.name << endl << source.city << endl <<source.state<< endl;
    source_hospital_data.close();

    // executes python script to retrieve longitude/longitude using geocoding
    system("python geocoding.py");

    ifstream lat_long("lat_long.txt");
    if(!lat_long.is_open()){
        cout << "Error: Unable to open long_lat file" << endl;
        return 1;
    }
    string source_longitude,source_latitude;
    if (getline(lat_long, source_latitude) && getline(lat_long, source_longitude)){
        source.latitude = stof(source_latitude);
        source.longitude = stof(source_longitude);
    }

cout << "Main.cpp file : " << source.latitude << endl << source.longitude << endl;
    return 0;
}
