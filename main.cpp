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
#include <cctype>

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
                throw invalid_argument("Too many data entries!");
            
            try
            {
                if (arg_counter == 1 or arg_counter == 4) // Validate parcel ID and volume
                {    
                    uint64_t char_counter = 0;
                    for (const char &c : entry)
                    {
                        char_counter++;
                        if (isspace(c) and char_counter == 1) // Ignore leading space character
                            continue;
                        else if (!isdigit(c))
                            throw invalid_argument("ID and volume must only contain digits!");
                    }

                    entry.erase(remove(entry.begin(), entry.end(), ' '), entry.end()); // Remove whitespace characters
                    parcel_row.push_back(entry);
                }

                else if (arg_counter == 2 or arg_counter == 3) // Validate the to and from city names
                {
                    string city = "";
                    uint64_t char_counter = 0;
                    for (const char &c : entry)
                    {
                        char_counter++;
                        if (isspace(c) and char_counter == 1) // Ignore leading space character
                            continue;
                        else if (!isalpha(c))
                            throw invalid_argument("City name must only contain alphabet characters!");
                        else
                            city.push_back(c);
                    }
                    parcel_row.push_back(city);
                }
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
                    string city = "";
                    for (const char &c : entry)
                    {
                        char_counter++;
                        if (isspace(c) and char_counter == 1) // Ignore leading space character
                            continue;
                        else if (!isalpha(c))
                            throw invalid_argument("City name must only contain alphabet characters!");
                        else
                            city.push_back(c);
                    }
                    map_row.push_back(city);
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
                    }

                    entry.erase(remove(entry.begin(), entry.end(), ' '), entry.end()); // Remove whitespace characters
                    map_row.push_back(entry);
                }
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
        /* Extract the distance between cities as an integer value. */
        try 
        {
            size_t pos;
            uint64_t distance = stoull(map_entry[2], &pos);
            if (pos < map_entry[2].size()) 
            {
                throw invalid_argument("Trailing characters after number!");
            }

            string city_1 = map_entry.at(0);
            string city_2 = map_entry.at(1);

            newMap.add_distance(city_1, city_2, distance); // Add the map entry into the distance map
        }

        catch (invalid_argument const &ex) 
        {
            cerr << "Invalid distance found: " << ex.what() << '\n';
            cout << correct_map_data;
            return -1;
        } 
        catch (out_of_range const &ex) 
        {
            cerr << "Distance number out of range: " << ex.what() << '\n';
            cout << correct_map_data;
            return -1;
        }
    }
    
    cout << "Created distance map for parcel delivery: \n";
    newMap.print_distance_map(); // Print the distance map to the terminal

    /* Use the data from the truck-data.csv to create truck objects and add them to a fleet of trucks. */
    
    fleet newfleet;
    vector<trucks> list_of_trucks; // Store the trucks read from the file
    for (vector<uint64_t> &truck_data : truck_file_contents)
    {
        try
        {
            trucks newtruck(truck_data[0], truck_data[1]);
            newfleet.add_truck(newtruck);
            list_of_trucks.push_back(newtruck);
        }
        catch(const exception &e)
        {
            cerr << e.what() << '\n';
        }
    }

    /* Use the data from the parcel-data.csv to create parcel objects. */
    vector<parcels> list_of_parcels; // Store the parcels read from the file
    for (vector<string> &parcel_data : parcel_file_contents)
    {
        /* Validate the parcel ID. */
        try 
        {
            size_t pos1;
            size_t pos2;
            uint64_t parcel_id = stoull(parcel_data[0], &pos1);
            uint64_t parcel_volume = stoull(parcel_data[3], &pos2);
            if (pos1 < parcel_data[0].size()) 
                throw invalid_argument("Trailing characters after parcel ID!");
            
            if (pos2 < parcel_data[3].size())
                throw invalid_argument("Trailing characters after parcel volume!");

            string from_city = parcel_data[1];
            string to_city = parcel_data[2];

            parcels newparcel(parcel_id, parcel_volume, from_city, to_city);
            list_of_parcels.push_back(newparcel); // Add the parcel to the storeage container
        }

        catch (invalid_argument const &ex) 
        {
            cerr << "Invalid number: " << ex.what() << '\n';
            cout << correct_parcel_data;
            return -1;
        } 
        catch (out_of_range const &ex) 
        {
            cerr << "Invalid number: " << ex.what() << '\n';
            cout << correct_parcel_data;
            return -1;
        }
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
