/**
 * @file main.cpp
 * @author Cassandra Masschelein
 * @brief A program that will read data from a map file, truck file, and parcel file and create various route schedules for delivery
 * @version 0.1
 * @date 2021-12-26
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/* C++ Header Files */
#include "domain.hpp"
#include "schedule.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

int main()
{
    /* Check that the input data files follow the specified format and contain valid data. */
    string correct_truck_data = "The truck data file must be formatted such that each line contains a truck ID followed by its capacity (in cm^3) with the data separated by a comma. Both numbers must be inputted as integers. An example line of data for a truck with ID: 101 and capacity: 150cm^3 would be \n 101, 150 \n";
    string correct_parcel_data = "The parcel data file must be formatted such that each line contains a parcel ID followed by its source city, destination city, and its volume (in cm^3). The data must be separated by a comma, and both the ID and volume must be integer values. An example line of data for a parcel with ID: 50, source city: Hamilton, destination city: Toronto, volume: 7cm^3 would be \n 50, Hamilton, Toronto, 7 \n";
    string correct_map_data = "The map data file must be formatted such that each line contains two cities followed by the distance between them (in km). The data must be separated by a comma and the distance must be an integer value. An example line of data for the distance between Hamilton and Toronto which have a distance of 69km would be \n Hamilton, Toronto, 69 \n";

    cout << "Reading file contents and preparing to create a delivery schedule for your parcels... \n";

    ifstream truck_data("truck-data.csv");
    if (!truck_data.is_open())
    {
        cout << "Error opening truck data file!";
        return -1;
    }

    /* Read csv file containing truck data row by row and split each row by comma delimeter. */
    vector<uint64_t> row;
    string line, entry; // A line is a row, and an entry is separated by a comma
    uint64_t line_number = 0;
    uint64_t arg_counter;
    vector<vector<uint64_t> > truck_file_contents; // Store the data in a vector

    while (getline(truck_data, line))
    {
        arg_counter = 0;
        line_number++;
        row.clear();
        stringstream str(line);

        while (getline(str, entry, ','))
        {
            arg_counter++;
            // Check that a line in the data file has the correct number of entries
            if (arg_counter > 2)
            {
                throw invalid_argument("Too many data entries!");
            }

            try 
            {
                size_t pos;
                uint64_t truck_data = stoull(entry, &pos);
                if (pos < entry.size()) 
                {
                    throw invalid_argument("Trailing characters after number!");
                }

                row.push_back(truck_data); // Add the comma separated value to a vector
            }

            catch (invalid_argument const &ex) 
            {
                cerr << "Invalid data entry: " << entry << " on line" << line_number << " of the truck-data.csv file. " << ex.what() << '\n';
                cout << correct_truck_data;
                return -1;
            } 
            catch (out_of_range const &ex) 
            {
                cerr << "Number out of range: " << entry << " on line" << line_number << " of truck-data.csv file." << '\n';
                cout << correct_truck_data;
                return -1;
            }
        }
        truck_file_contents.push_back(row); // Add the row vector to the data vector
    }

    cout << "Truck data has been successfully read. \n";
    truck_data.close();

    /* Read csv file containing parcel data row by row and split each row by comma delimeter. */
    ifstream parcel_data("parcel-data.csv");
    if (!parcel_data.is_open())
    {
        cout << "Error opening parcel data file!";
        return -1;
    }

    vector<string> parcel_row;
    line_number = 0;
    vector<vector<string> > parcel_file_contents; // Store the data in a vector

    while (getline(parcel_data, line))
    {
        arg_counter = 0;
        line_number++;
        parcel_row.clear();
        stringstream str(line);

        while (getline(str, entry, ','))
        {
            arg_counter++;
            // Check that a line in the data file has the correct number of entries
            if (arg_counter > 4)
            {
                throw invalid_argument("Too many data entries!");
            }

            try
            {
                if (arg_counter == 1) // Validate the parcel ID
                {
                    uint64_t char_counter = 0;
                    for (const char &c : entry)
                    {
                        char_counter++;
                        if (isspace(c) and char_counter == 1) // Ignore leading space character
                            continue;
                        else if (!isdigit(c))
                            throw invalid_argument("ID value must only contain digits!");
                        if (char_counter > 6)
                            throw out_of_range("ID values should be small and unique.");
                    }
                }

                else if (arg_counter == 4) // Validate the parcel volume
                {
                    uint64_t char_counter = 0;
                    for (const char &c : entry)
                    {
                        char_counter++;
                        if (isspace(c) and char_counter == 1) // Ignore leading space character
                            continue;
                        else if (!isdigit(c))
                            throw invalid_argument("Volume of the parcel must be an integer value!");
                        if (char_counter > 5)
                            throw out_of_range("Truck capacity must be a realistic value and in units cm^3.");
                    }
                }

                else if (arg_counter == 2 or arg_counter == 3) // Validate the to and from city names
                {
                    uint64_t char_counter = 0;
                    for (const char &c : entry)
                    {
                        char_counter++;
                        if (isspace(c) and char_counter == 1) // Ignore leading space character
                            continue;
                        else if (!isalpha(c))
                            throw invalid_argument("City name must only contain alphabet characters!");
                    }
                }

                parcel_row.push_back(entry); // Add the comma separated value to a vector
            }
            catch(const invalid_argument &ex)
            {
                cerr << "Invalid data entry: " << entry << " found on line " << line_number << " of the parcel-data.csv file. " << ex.what() << '\n';
                cout << correct_parcel_data;
                return -1;
            }

            catch (const out_of_range &ex) 
            {
                cerr << "Number out of range: " << entry << " found on line" << line_number << " of the parcel-data.csv file. " << ex.what() << '\n';
                cout << correct_parcel_data;
                return -1;
            }
        }
        parcel_file_contents.push_back(parcel_row); // Add the row vector to the data vector
    }

    cout << "Parcel data has been successfully read. \n";
    parcel_data.close();

    ifstream map_data("map-data.csv");
    if (!map_data.is_open())
    {
        cout << "Error opening map data file!";
        return -1;
    }

    /* Read csv file containing map data row by row and split each row by comma delimeter. */
    vector<string> map_row;
    line_number = 0;
    vector<vector<string> > map_file_contents; // Store the data in a vector

    while (getline(map_data, line))
    {
        arg_counter = 0;
        line_number++;
        map_row.clear();
        stringstream str(line);

        while (getline(str, entry, ','))
        {
            arg_counter++;
            // Check that a line in the data file has the correct number of entries
            if (arg_counter > 3)
            {
                throw invalid_argument("Too many data entries!");
            }

            try
            {
                if (arg_counter == 1 or arg_counter == 2) // Validate the city names
                {
                    uint64_t char_counter = 0;
                    for (const char &c : entry)
                    {
                        char_counter++;
                        if (isspace(c) and char_counter == 1) // Ignore leading space character
                            continue;
                        else if (!isalpha(c))
                            throw invalid_argument("City name must only contain alphabet characters!");
                    }
                }

                if (arg_counter == 3) // Validate the distance
                {
                    uint64_t char_counter = 0;
                    for (const char &c : entry)
                    {
                        char_counter++;
                        if (isspace(c) and char_counter == 1) // Ignore leading space character
                            continue;
                        else if (!isdigit(c))
                            throw invalid_argument("Distance value must only contain digits!");
                        if (char_counter > 4)
                            throw out_of_range("Cities in the map must be in Southern Ontario and thus within driving distance. Distances are in kilometers.");
                    }
                }

                map_row.push_back(entry); // Add the comma separated value to a vector
            }
            catch(const invalid_argument &ex)
            {
                cerr << "Invalid data entry:" << entry << "Found on line " << line_number << " of the map-data.csv file. " << ex.what() << '\n';
                cout << correct_map_data;
                return -1;
            }

            catch (const out_of_range &ex) 
            {
                cerr << "Number out of range: " << entry << " Found on line" << line_number << " of the map-data.csv file. " << ex.what() << '\n';
                cout << correct_map_data;
                return -1;
            }
        }
        map_file_contents.push_back(map_row); // Add the row vector to the data vector
    }

    cout << "Map data has been successfully read. \n";
    map_data.close();
    
    /* Run some scheduling experiments using the data that was read from the input files. */
    /* Use the data from the map-data.csv to create a distanceMap object and add corresponding map entries. */
    distanceMap newMap;
    for (vector<string> &map_entry : map_file_contents)
    {
        string city_1 = map_entry.at(0);
        string city_2 = map_entry.at(1);
        /* Extract the distance between citites as an integer value. */
        try 
        {
            size_t pos;
            uint64_t x = stoull(map_entry[2], &pos);
            if (pos < entry.size()) 
            {
                throw invalid_argument("Trailing characters after number!");
            }
            if (x > 1000)
            {
                throw invalid_argument("Distance is too far, cities must be within Southern Ontario.");
            }
        }

        catch (invalid_argument const &ex) 
        {
            cerr << "Invalid distance: " << ex.what() << '\n';
            cout << correct_truck_data;
            return -1;
        } 
        catch (out_of_range const &ex) 
        {
            cerr << "Number out of range: " << ex.what() << '\n';
            cout << correct_truck_data;
            return -1;
        }

        size_t pos;
        uint64_t distance = stoull(map_entry[2], &pos);

        city_1.erase(remove(map_entry[0].begin(), map_entry[0].end(), ' '), map_entry[0].end()); // Remove any extra whitespace characters
        city_2.erase(remove(map_entry[1].begin(), map_entry[1].end(), ' '), map_entry[1].end()); // Remove any extra whitespace characters
        newMap.add_distance(city_1, city_2, distance); // Add the map entry into the distance map
    }
    
    cout << "Created distance map for parcel delivery: \n";
    newMap.print_distance_map(); // Print the distance map to the terminal

    /* Use the data from the truck-data.csv to create truck objects and add them to a fleet of trucks. */
    fleet newfleet;
    vector<trucks> list_of_trucks; // Store the trucks read from the file
    for (vector<uint64_t> &truck_data : truck_file_contents)
    {
        trucks newtruck(truck_data[0], truck_data[1], COMMON_DEPOT);
        newfleet.add_truck(newtruck);
        list_of_trucks.push_back(newtruck);
    }

    /* Use the data from the parcel-data.csv to create parcel objects. */
    vector<parcels> list_of_parcels; // Store the parcels read from the file
    for (vector<string> &parcel_data : parcel_file_contents)
    {
        string from_city = parcel_data[1];
        string to_city = parcel_data[2];
        /* Validate the parcel ID. */
        try 
        {
            size_t pos;
            uint64_t y = stoull(parcel_data[0], &pos);
            if (pos < entry.size()) 
            {
                throw invalid_argument("Trailing characters after number!");
            }
            if (y > 100000) // The ID is not valid
            {
                throw invalid_argument("Parcel ID is too long, pick something small and unique.");
            }
        }

        catch (invalid_argument const &ex) 
        {
            cerr << "Invalid parcel ID: " << ex.what() << '\n';
            cout << correct_parcel_data;
            return -1;
        } 
        catch (out_of_range const &ex) 
        {
            cerr << "Parcel ID out of range: " << ex.what() << '\n';
            cout << correct_parcel_data;
            return -1;
        }

        /* Validate the parcel volume. */
        try 
        {
            size_t pos;
            uint64_t z = stoull(parcel_data[3], &pos);
            if (pos < entry.size()) 
            {
                throw invalid_argument("Trailing characters after number!");
            }
            if (z > 100000) // The volume is not valid
            {
                throw invalid_argument("Parcel volume must be in cm^3. Check your units.");
            }
        }

        catch (invalid_argument const &ex) 
        {
            cerr << "Invalid parcel volume: " << ex.what() << '\n';
            cout << correct_parcel_data;
            return -1;
        } 
        catch (out_of_range const &ex) 
        {
            cerr << "Parcel volume out of range: " << ex.what() << '\n';
            cout << correct_parcel_data;
            return -1;
        }

        size_t pos;
        uint64_t parcel_id = stoull(parcel_data[3], &pos);
        uint64_t parcel_volume = stoull(parcel_data[0], &pos);

        from_city.erase(remove(parcel_data[1].begin(), parcel_data[1].end(), ' '), parcel_data[1].end()); // Remove any whitespace characters
        to_city.erase(remove(parcel_data[2].begin(), parcel_data[2].end(), ' '), parcel_data[2].end()); // Remove any whitespace characters

        parcels newparcel(parcel_id, parcel_volume, from_city, to_city);
        list_of_parcels.push_back(newparcel); // Add the parcel to the storeage container
    }

    cout << "Generating possible delivery schedules to deliver your parcels...\n";

    /* Generate the file for writing the scheduling algorithm statistics to. */
    ofstream route_stats("route-stats.csv");
    if (!route_stats.is_open())
    {
        cout << "Error opening output file for route scheduling statistics!";
        return -1;
    }

    mostparcelScheduler pack_most_parcels(list_of_parcels, list_of_trucks); // Create clones to use in each scheduling algorithm
    vector<parcels> unpacked_parcels = pack_most_parcels.schedule();

    /* Write the statistics to the file. */
    route_stats << "Scheduler" << "," << "Free Vol in Used Trucks" << "," << "Avg Capacity Used" << "," << "Std Dev Avg Capacity" << "," << "Avg Dist" << "," << "Std Dev Dist" << "\n";
    route_stats << "Most Parcels" << "," << newfleet.free_vol_in_used_trucks() << "," << newfleet.avg_capacity_used() << "," << newfleet.std_dev_capacity_used() << "," << newfleet.avg_distance_travelled(newMap) << "," << newfleet.std_dev_capacity_used() << "/n";
    newfleet.print_fleet(); // Print out the fleet schedule per this scheduling algorithm
    route_stats.close();
}
