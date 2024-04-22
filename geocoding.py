import requests

def get_coordinates(hospital_name, city, state, api_key):
    base_url = "https://maps.googleapis.com/maps/api/geocode/json"
    params = {
        "address": f"{hospital_name}, {city}, {state}",
        "key": api_key
    }
    response = requests.get(base_url, params=params)
    if response.status_code == 200:
        data = response.json()
        if data['status'] == 'OK':
            location = data['results'][0]['geometry']['location']
            latitude = location['lat']
            longitude = location['lng']
            return latitude, longitude
        else:
            print("Geocoding API request failed.")
    else:
        print("Error accessing the Geocoding API.")

with open('source_hospital_data.txt', 'r') as file:
    lines = file.readlines()
    hospital_name = lines[0].strip()
    city = lines[1].strip()
    state = lines[2].strip()

api_key = "AIzaSyAkpts4X2Oj-S6ogvRmUaV_fjcoS_FJWCs"
latitude, longitude = get_coordinates(hospital_name, city, state, api_key)
f = open("lat_long.txt", "w");
f.write(str(latitude))
f.write("\n")
f.write(str(longitude))
f.close()
