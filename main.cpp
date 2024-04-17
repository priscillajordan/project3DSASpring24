    #include <iostream>
    #include <fstream>
    #include <sstream>
    #include <string>
    #include <vector>

    using namespace std;

    class Hospital{
        string name;
        string city;
        int longitude;
        int latitude;
    public:

    Hospital(){
        name = "empty";
        city = "NaN";
        longitude = 0;
        latitude = 0;
    }
    Hospital(string name, string city, int longitude, int latitude){
        this->name = name;
        this->city = city;
        this->latitude = latitude;
        this->longitude = longitude;
    }

    ~Hospital(){}
        
    };

    int main() {
    // Open the data set file
    ifstream file;
    file.open("hospitals.csv");
    if(!file.is_open()){
        cout << "file not open" << endl;
    }

    // Create a vector of vectors to store the data
    vector<vector<string>> data;

    // Read the data from the file
    string line;
    while (getline(file, line)) {
        // Split the line into tokens
        vector<string> tokens;
        istringstream iss(line);
        string token;
        while (getline(iss, token, ',')) {
        tokens.push_back(token);
        }

        // Add the tokens to the data vector
        data.push_back(tokens);
    }

    // Close the data set file
    file.close();

    // Print the data
    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[i].size(); j++) {
        cout << data[i][j] << " ";
        }
        cout << endl;
    }
    cout << "printing done" << endl;


    return 0;
    }
