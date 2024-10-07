#include <iostream>
#include <string>
#include "src/get_data.h"
#include "src/planets.h"  // Include the planets header
#include <vector>
#include <cmath>
#include <cstdlib>

using namespace std;

// ** Class Definitions **

// Base class for Space Bodies (e.g., Planets, Asteroids)
class SpaceBody {
public:
    SpaceBody(const string& name, double diameter, double mass) 
        : name(name), diameter(diameter), mass(mass) {}

    virtual void printInfo() const {
        cout << "Name: " << name << ", Diameter: " << diameter << " km, Mass: " << mass << " kg" << endl;
    }

    // Calculate surface gravity
    double calculateSurfaceGravity() const {
        const double G = 6.67430e-11;  // Gravitational constant (m^3 kg^-1 s^-2)
        double radius_m = (diameter * 1000) / 2.0;  // Convert km to meters and get radius
        return (G * mass) / (radius_m * radius_m);  // Surface gravity in m/s^2
    }

    // Destructor
    virtual ~SpaceBody() {
        cout << "Space body " << name << " destroyed." << endl;
    }

protected:
    string name;
    double diameter;  // in kilometers
    double mass;      // in kilograms
};

// Derived class for Planets
class Planet : public SpaceBody {
public:
    Planet(const string& name, double diameter, double mass) 
        : SpaceBody(name, diameter, mass) {}

    void printInfo() const override {
        cout << "Planet Name: " << name << ", Mass: " << mass << " kg, Diameter: " << diameter << " km" << endl;
        cout << "Surface Gravity: " << calculateSurfaceGravity() << " m/s^2" << endl;
        cout << "Escape Velocity: " << calculateEscapeVelocity() << " km/s" << endl;
    }

    // Calculate escape velocity in km/s
    double calculateEscapeVelocity() const {
        const double G = 6.67430e-11;  // Gravitational constant (m^3 kg^-1 s^-2)
        double radiusMeters = (diameter * 1000) / 2.0;  // Convert km to meters and get radius
        double escapeVelocity_m_s = sqrt((2 * G * mass) / radiusMeters);  // Escape velocity in m/s
        return escapeVelocity_m_s / 1000.0;  // Convert to km/s
    }

    // Destructor
    ~Planet() {
        cout << "Planet " << name << " destroyed." << endl;
    }
};

// Derived class for Asteroids
class Asteroid : public SpaceBody {
public:
    // Copy constructor
    Asteroid(const Asteroid& other) 
        : SpaceBody(other.name, other.diameter, other.mass),
          id(other.id),
          nasa_jpl_url(other.nasa_jpl_url),
          absolute_magnitude(other.absolute_magnitude),
          minDiameterKm(other.minDiameterKm),
          maxDiameterKm(other.maxDiameterKm),
          isDangerous(other.isDangerous),
          closeApproachDate(other.closeApproachDate),
          relativeVelocityKmPerS(other.relativeVelocityKmPerS),
          missDistanceKm(other.missDistanceKm) {
        cout << "Asteroid " << name << " copied." << endl;
    }

    // Asteroid constructor, extracts data from the JSON object
    Asteroid(const json& asteroidData) 
        : SpaceBody(
            asteroidData["name"], 
            asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_min"].get<double>(), 
            calculateMass(asteroidData)
          ),
          id(asteroidData["id"]),
          nasa_jpl_url(asteroidData["nasa_jpl_url"]),
          absolute_magnitude(asteroidData["absolute_magnitude_h"]),
          isDangerous(asteroidData["is_potentially_hazardous_asteroid"])
    {
        // Extract diameter information
        auto diameter = asteroidData["estimated_diameter"];
        minDiameterKm = diameter["kilometers"]["estimated_diameter_min"].get<double>();
        maxDiameterKm = diameter["kilometers"]["estimated_diameter_max"].get<double>();

        // Extract close approach data
        auto close_approach = asteroidData["close_approach_data"][0];
        closeApproachDate = close_approach["close_approach_date"];
        relativeVelocityKmPerS = stod(close_approach["relative_velocity"]["kilometers_per_second"].get<string>());
        missDistanceKm = stod(close_approach["miss_distance"]["kilometers"].get<string>());
    }

    void printInfo() const override {
        cout << "Asteroid ID: " << id << endl;
        cout << "Name: " << name << endl;
        cout << "NASA JPL URL: " << nasa_jpl_url << endl;
        cout << "Absolute Magnitude (H): " << absolute_magnitude << endl;
        cout << "Diameter (Min): " << minDiameterKm << " km, Max: " << maxDiameterKm << " km" << endl;
        cout << "Is Potentially Hazardous: " << (isDangerous ? "Yes" : "No") << endl;
        cout << "Close Approach Date: " << closeApproachDate << endl;
        cout << "Relative Velocity: " << relativeVelocityKmPerS << " km/s" << endl;
        cout << "Miss Distance: " << missDistanceKm << " km" << endl;
        cout << "Mass: " << mass << " kg" << endl;
        cout << "Surface Gravity: " << calculateSurfaceGravity() << " m/s^2" << endl;
        cout << "Impact Energy: " << calculateImpactEnergy() << " megatons of TNT" << endl;
    }

    // Calculate impact energy in megatons of TNT
    double calculateImpactEnergy() const {
        // Kinetic energy formula: E = 0.5 * mass * velocity^2
        double velocity_m_s = relativeVelocityKmPerS * 1000.0;  // Convert km/s to m/s
        double energy_joules = 0.5 * mass * pow(velocity_m_s, 2);  // Energy in Joules

        // Convert Joules to megatons of TNT (1 megaton TNT = 4.184e15 J)
        double energy_megatons = energy_joules / 4.184e15;

        return energy_megatons;
    }

    // Operator overload for adding two Asteroids
    Asteroid operator+(const Asteroid& other) const {
        // Create a copy of the current asteroid
        Asteroid combinedAsteroid(*this);

        // Modify necessary fields
        combinedAsteroid.name = name + " & " + other.name; // Combine names
        // Combine diameters
        combinedAsteroid.minDiameterKm += other.minDiameterKm;
        combinedAsteroid.maxDiameterKm += other.maxDiameterKm;
        // Combine masses
        combinedAsteroid.mass += other.mass;
        // Combine relative velocities
        combinedAsteroid.relativeVelocityKmPerS += other.relativeVelocityKmPerS;
        // Combine miss distances
        combinedAsteroid.missDistanceKm += other.missDistanceKm;
        // Update the potentially hazardous status
        combinedAsteroid.isDangerous =
            ((combinedAsteroid.minDiameterKm > 280) || (combinedAsteroid.relativeVelocityKmPerS > 5.0));

        return combinedAsteroid;
    }

    // Destructor
    ~Asteroid() {
        cout << "Asteroid " << name << " destroyed." << endl;
    }

private:
    // Member variables for the asteroid
    string id;
    string nasa_jpl_url;
    double absolute_magnitude;
    double minDiameterKm;
    double maxDiameterKm;
    bool isDangerous;
    string closeApproachDate;
    double relativeVelocityKmPerS;
    double missDistanceKm;

    // Static helper function to calculate mass
    static double calculateMass(const json& asteroidData) {
        // Assume density of asteroid (in kg/m^3)
        const double density = 3000.0;  // Example density in kg/m^3

        double diameterMin_m = asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_min"].get<double>() * 1000.0; // km to m
        double diameterMax_m = asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_max"].get<double>() * 1000.0; // km to m

        double radiusMin = diameterMin_m / 2.0;
        double radiusMax = diameterMax_m / 2.0;

        double volumeMin = (4.0 / 3.0) * M_PI * pow(radiusMin, 3);
        double volumeMax = (4.0 / 3.0) * M_PI * pow(radiusMax, 3);

        double avgVolume = (volumeMin + volumeMax) / 2.0;

        double mass = density * avgVolume;  // mass in kg

        return mass;
    }
};

int main() {
    loadEnvFile(".env");

    bool continueAnalyzing = true;
    while (continueAnalyzing) {
        // Ask the user for a single date
        string selectedDate;
        cout << "\n\nWelcome to the NEO Analyzer!" << endl;
        cout << "Enter a date (YYYY-MM-DD) to search for NEOs: ";
        cin >> selectedDate;

        const char* apiKeyEnv = getenv("API_KEY");  // Get API key from environment variable
        string apiKey = apiKeyEnv ? apiKeyEnv : "";  // If environment variable is missing, use empty string

        if (apiKey.empty()) {
            cerr << "API key is missing. Please set the API_KEY environment variable." << endl;
            return 1;  // Exit program if API key is not set
        }

        // Declare jsonData and selectedNeoJson outside the if-else blocks
        json jsonData;        // JSON object to store loaded data
        json selectedNeoJson; // To hold the selected NEO data

        // Fetch NEO data for the selected date and the API key
        string neo_data = fetch_neo_data(selectedDate, apiKey);

        if (neo_data.empty()) {
            cerr << "Failed to fetch data from NASA API. Loading data from file..." << endl;

            // If fetching from API fails, load from file
            if (!load_from_file(jsonData, "data.json")) {
                cerr << "Failed to load data from file." << endl;
                return 1;  // Exit if both API and file loading fail
            }

            selectedNeoJson = process_neo_data(jsonData, selectedDate);  // Process data from file

        } else {
            try {
                jsonData = json::parse(neo_data);  // Parse the JSON response
                selectedNeoJson = process_neo_data(jsonData, selectedDate);  // Process the data fetched from API
            } catch (const exception& e) {
                cerr << "Error parsing data: " << e.what() << endl;
                return 1;
            }
        }

        // Check if a valid NEO was selected
        if (!selectedNeoJson.empty()) {
            try {
                // Create asteroid class object
                Asteroid asteroid1(selectedNeoJson);

                bool asteroidMenu = true;
                while (asteroidMenu) {
                    // Display action menu
                    cout << "\nPlease select an option:\n";
                    cout << "1. Print all information about the asteroid.\n";
                    cout << "2. Calculate and display the surface gravity.\n";
                    cout << "3. Calculate and display the impact energy.\n";
                    cout << "4. Combine this asteroid with another asteroid.\n";
                    cout << "5. Analyze planets in the solar system.\n";
                    cout << "6. Exit or Return to main menu.\n";
                    cout << "Enter your choice: ";

                    int choice;
                    cin >> choice;

                    switch (choice) {
                        case 1:
                            cout << "\n--- Asteroid Information ---\n";
                            asteroid1.printInfo();
                            break;
                        case 2:
                            cout << "\nSurface Gravity: " << asteroid1.calculateSurfaceGravity() << " m/s^2\n";
                            break;
                        case 3:
                            cout << "\nImpact Energy: " << asteroid1.calculateImpactEnergy() << " megatons of TNT\n";
                            break;
                        case 4: {
                            cout << "\n--- Asteroid Information ---\n";
                            asteroid1.printInfo();
                            string selectedDate2;
                            cout << "\nEnter a second date (YYYY-MM-DD) to search for NEOs: ";
                            cin >> selectedDate2;
                            string neo_data2 = fetch_neo_data(selectedDate2, apiKey);

                            if (!neo_data2.empty()) {
                                jsonData = json::parse(neo_data2);
                                json selectedNeoJson2 = process_neo_data(jsonData, selectedDate2);

                                if (!selectedNeoJson2.empty()) {
                                    Asteroid asteroid2(selectedNeoJson2);
                                    cout << "\n--- Second Asteroid Information ---\n";
                                    asteroid2.printInfo();

                                    // Combine the two asteroids
                                    cout << "\nCombining the two asteroids...\n";
                                    Asteroid combinedAsteroid = asteroid1 + asteroid2;
                                    cout << "\n--- Combined Asteroid Information ---\n";
                                    combinedAsteroid.printInfo();
                                } else {
                                    cout << "No asteroid selected for the second date.\n";
                                }
                            } else {
                                cout << "Failed to fetch data for the second date from NASA API.\n";
                            }
                            break;
                        }
                        case 5:
                            cout << "\n--- Predefined Planets Information ---\n";
                            for (const auto& pdata : predefinedPlanets) {
                                Planet planet(pdata.name, pdata.diameter, pdata.mass);
                                planet.printInfo();
                                cout << "------------------------------\n";
                            }
                            break;
                        case 6:
                            asteroidMenu = false;  // Exit the asteroid menu
                            break;
                        default:
                            cout << "Invalid choice. Please select a valid option.\n";
                    }

                    if (asteroidMenu) {
                        // Ask if the user wants to continue with the current asteroid
                        cout << "\nDo you want to perform another action on this asteroid? (y/n): ";
                        char continueChoice;
                        cin >> continueChoice;
                        if (continueChoice == 'n' || continueChoice == 'N') {
                            asteroidMenu = false;
                        }
                    }
                }

            } catch (const exception& e) {
                cerr << "Error creating Asteroid object: " << e.what() << endl;
            }
        } else {
            cout << "No asteroid selected.\n";
        }

        // Ask if the user wants to analyze another asteroid
        cout << "\nDo you want to analyze another asteroid? (y/n): ";
        char mainChoice;
        cin >> mainChoice;
        if (mainChoice == 'n' || mainChoice == 'N') {
            continueAnalyzing = false;
            cout << "Exiting the NEO Analyzer. Goodbye!\n";
        }
    }

    return 0;
}
