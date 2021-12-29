/**
 * @file DeliverParcels.cpp
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

    vector<uint64_t> row;
    string line, entry;
    uint64_t line_number = 0;
    uint64_t arg_counter;
    vector<vector<uint64_t> > truck_file_contents;

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

                row.push_back(truck_data);
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
        truck_file_contents.push_back(row);
    }

    cout << "Truck data has been successfully read. \n";
    truck_data.close();

    ifstream parcel_data("parcel-data.csv");
    if (!parcel_data.is_open())
    {
        cout << "Error opening parcel data file!";
        return -1;
    }

    vector<string> parcel_row;
    line_number = 0;
    vector<vector<string> > parcel_file_contents;

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
                if (arg_counter == 1)
                {
                    uint64_t char_counter = 0;
                    for (const char &c : entry)
                    {
                        char_counter++;
                        if (isspace(c) and char_counter == 1)
                            continue;
                        else if (!isdigit(c))
                            throw invalid_argument("ID value must only contain digits!");
                        if (char_counter > 6)
                            throw out_of_range("ID values should be small and unique.");
                    }
                }

                else if (arg_counter == 4)
                {
                    uint64_t char_counter = 0;
                    for (const char &c : entry)
                    {
                        char_counter++;
                        if (isspace(c) and char_counter == 1)
                            continue;
                        else if (!isdigit(c))
                            throw invalid_argument("Volume of the parcel must be an integer value!");
                        if (char_counter > 5)
                            throw out_of_range("Truck capacity must be a realistic value and in units cm^3.");
                    }
                }

                else if (arg_counter == 2 or arg_counter == 3)
                {
                    uint64_t char_counter = 0;
                    for (const char &c : entry)
                    {
                        char_counter++;
                        if (isspace(c) and char_counter == 1)
                            continue;
                        else if (!isalpha(c))
                            throw invalid_argument("City name must only contain alphabet characters!");
                    }
                }

                parcel_row.push_back(entry); // Add the entry to the vector as a string for now
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
        parcel_file_contents.push_back(parcel_row);
    }

    cout << "Parcel data has been successfully read. \n";
    parcel_data.close();

    ifstream map_data("map-data.csv");
    if (!map_data.is_open())
    {
        cout << "Error opening map data file!";
        return -1;
    }

    vector<string> map_row;
    line_number = 0;
    vector<vector<string> > map_file_contents;

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
                if (arg_counter == 1 or arg_counter == 2)
                {
                    uint64_t char_counter = 0;
                    for (const char &c : entry)
                    {
                        char_counter++;
                        if (isspace(c) and char_counter == 1)
                            continue;
                        else if (!isalpha(c))
                            throw invalid_argument("City name must only contain alphabet characters!");
                    }
                }

                if (arg_counter == 3)
                {
                    uint64_t char_counter = 0;
                    for (const char &c : entry)
                    {
                        char_counter++;
                        if (isspace(c) and char_counter == 1)
                            continue;
                        else if (!isdigit(c))
                            throw invalid_argument("Distance value must only contain digits!");
                        if (char_counter > 4)
                            throw out_of_range("Cities in the map must be in Southern Ontario and thus within driving distance. Distances are in kilometers.");
                    }
                }

                map_row.push_back(entry);
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
        map_file_contents.push_back(map_row);
    }

    cout << "Map data has been successfully read. \n";
    map_data.close();
    
    // run some experiments here

    // create the objects needed - parcels, trucks, fleet
    // as you make the parcels - add them to a priority queue one by one and sort along the way - like check last one and keep going one back until prioroty it right
    // do priority by ordered cities close to far from source
    // add to truck by first making temp list of trucks with enough space. then go through and give prioroty to a truck with destination already in route
    // do error checking for the objects to ensure class invariance
    // schedule the deliveries based on the different priority algorithms
    // run experiments on the algorithm performance
    // write data to output file

    // create distance map object
    distanceMap newMap;
    for (uint64_t i = 0; i < map_file_contents.size(); i++)
    {
        string city_1 = map_file_contents[i][0];
        string city_2 = map_file_contents[i][1];
        try 
        {
            size_t pos;
            uint64_t x = stoull(map_file_contents[i][2], &pos);
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
        uint64_t distance = stoull(map_file_contents[i][2], &pos);

        city_1.erase(remove(map_file_contents[i][0].begin(), map_file_contents[i][0].end(), ' '), map_file_contents[i][0].end());
        city_2.erase(remove(map_file_contents[i][1].begin(), map_file_contents[i][1].end(), ' '), map_file_contents[i][1].end());
        newMap.add_distance(city_1, city_2, distance);
    }

    /*

    for (vector<string> &map_entry : map_file_contents)
    {
        string city_1 = map_entry.at(0);
        string city_2 = map_entry.at(1);
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

        city_1.erase(remove(map_entry[0].begin(), map_entry[0].end(), ' '), map_entry[0].end());
        city_2.erase(remove(map_entry[1].begin(), map_entry[1].end(), ' '), map_entry[1].end());
        newMap.add_distance(city_1, city_2, distance);
    }
    */

    cout << "Created distance map for parcel delivery: \n";
    newMap.print_distance_map();

    // create the parcel objects, truck objects, and the fleet of trucks
    fleet newfleet;
    vector<trucks> list_of_trucks;
    for (vector<uint64_t> &truck_data : truck_file_contents)
    {
        trucks newtruck(truck_data[0], truck_data[1], COMMON_DEPOT);
        newfleet.add_truck(newtruck);
        list_of_trucks.push_back(newtruck);
    }

    // create parcel objects
    vector<parcels> list_of_parcels;
    for (vector<string> &parcel_data : parcel_file_contents)
    {
        string from_city = parcel_data[1];
        string to_city = parcel_data[2];
        try 
        {
            size_t pos;
            uint64_t y = stoull(parcel_data[0], &pos);
            if (pos < entry.size()) 
            {
                throw invalid_argument("Trailing characters after number!");
            }
            if (y > 100000)
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
        try 
        {
            size_t pos;
            uint64_t z = stoull(parcel_data[3], &pos);
            if (pos < entry.size()) 
            {
                throw invalid_argument("Trailing characters after number!");
            }
            if (z > 100000)
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

        from_city.erase(remove(parcel_data[1].begin(), parcel_data[1].end(), ' '), parcel_data[1].end());
        to_city.erase(remove(parcel_data[2].begin(), parcel_data[2].end(), ' '), parcel_data[2].end());

        parcels newparcel(parcel_id, parcel_volume, from_city, to_city);
        list_of_parcels.push_back(newparcel);
    }

    cout << "Generating possible delivery schedules to deliver your parcels...\n";

    ofstream route_stats("route-stats.csv");
    if (!route_stats.is_open())
    {
        cout << "Error opening output file for route scheduling statistics!";
        return -1;
    }

    mostparcelScheduler pack_most_parcels(list_of_parcels, list_of_trucks);
    vector<parcels> unpacked_parcels = pack_most_parcels.schedule();

    // Look at scheduling stats

    route_stats << "Scheduler" << "," << "Free Vol in Used Trucks" << "," << "Avg Capacity Used" << "," << "Std Dev Avg Capacity" << "," << "Avg Dist" << "," << "Std Dev Dist" << "\n";
    route_stats << "Most Parcels" << "," << newfleet.free_vol_in_used_trucks() << "," << newfleet.avg_capacity_used() << "," << newfleet.std_dev_capacity_used() << "," << newfleet.avg_distance_travelled(newMap) << "," << newfleet.std_dev_capacity_used() << "/n";

    //create schedules and write to file - also print out the trucks and routes for the fleet? maybe just print to terminal

    route_stats.close();

}