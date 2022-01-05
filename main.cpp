/**
 * @file main.cpp
 * @author Cassandra Masschelein
 * @brief A program that will read data from a map file, truck file, and parcel file and create various route schedules for delivery. User must input COMMON DEPOT into the program as a main argument
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

/**
 * @brief Load trucks onto the fleet of trucks that are scheduled for delivering parcels
 * 
 * @param list_of_trucks The list of trucks to be added to the fleet
 * @param newfleet A fleet object
 */
void load_fleet(const vector<trucks> &list_of_trucks, fleet &newfleet)
{
    for (const trucks &truck : list_of_trucks)
        newfleet.add_truck(truck);
}

int main(int argc, char* argv[])
{
    /* Check that the input data files follow the specified format and contain valid data. */
    string correct_common_depot = "The common depot for all the trucks must be a single city name. This name must be spelled properly and it must start with a capital letter. For example if your desired common depot was Toronto you would simply run the program with the argument: Toronto \n";
    string correct_truck_data = "The truck data file must be formatted such that each line contains a truck ID followed by its capacity (in cm^3) with the data separated by a comma. Both numbers must be inputted as integers. An example line of data for a truck with ID: 101 and capacity: 150cm^3 would be \n 101, 150 \n";
    string correct_parcel_data = "The parcel data file must be formatted such that each line contains a parcel ID followed by its source city, destination city, and its volume (in cm^3). The data must be separated by a comma, and both the ID and volume must be integer values. An example line of data for a parcel with ID: 50, source city: Hamilton, destination city: Toronto, volume: 7cm^3 would be \n 50, Hamilton, Toronto, 7 \n";
    string correct_map_data = "The map data file must be formatted such that each line contains two cities followed by the distance between them (in km). The data must be separated by a comma and the distance must be an integer value. An example line of data for the distance between Hamilton and Toronto which have a distance of 69km would be \n Hamilton, Toronto, 69 \n";

    /* Validate program argument. */
    if (argc < 2)
    {
        cout << "This program must take the common depot as the single argument! Please include the depot location! \n";
        cout << correct_common_depot;
        return -1;
    }
    else if (argc > 2)
    {
        cout << "This program only takes one argument! Please only include the name of the depot location. \n";
        cout << correct_common_depot;
        return -1;
    }
    else
    {
        try
        {
            string common_depot = argv[1];
            uint64_t char_counter = 0;
            for (const char &c : common_depot)
            {
                char_counter++;
                if (char_counter == 1 and not (isalpha(c) and isupper(c)))
                    throw invalid_argument("Depot name must start with a capital letter!");
                if (char_counter > 1 and not (isalpha(c) and islower(c)))
                    throw invalid_argument("Depot name must be a proper city name that is capitalized!");
            }
        }
        catch (const invalid_argument &ex) 
        {
            cerr << "Invalid argument: " << ex.what() << '\n';
            cout << correct_common_depot;
            return -1;
        } 
    }

    /**
     * @brief The common depot for all the trucks to start their routes from
     * 
     */
    string COMMON_DEPOT = argv[1];
    cout << "Reading file contents and preparing to create a delivery schedule for your parcels... \n";

    /* Open the data file containing the truck information. */
    ifstream truck_data("truck-data.csv");
    if (!truck_data.is_open())
    {
        cout << "Error opening truck data file!";
        return -1;
    }

    /* Read csv file containing truck data row by row and split each row by comma delimeter. */
    vector<uint64_t> row;
    string line, entry; // A line is a row, and an entry is a comma separated value
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
            if (arg_counter > 2) // Check that a line in the data file has the correct number of entries
                throw invalid_argument("Too many data entries!");

            try 
            {
                size_t pos;
                uint64_t truck_data = stoull(entry, &pos);
                if (pos < entry.size()) 
                    throw invalid_argument("Trailing characters after number!");

                row.push_back(truck_data); // Add the comma separated value to the row vector
            }

            catch (const invalid_argument &ex) 
            {
                cerr << "Invalid data entry: " << entry << " on line" << line_number << " of the truck-data.csv file. " << ex.what() << '\n';
                cout << correct_truck_data;
                return -1;
            } 
            catch (const out_of_range &ex) 
            {
                cerr << "Number out of range: " << entry << " on line" << line_number << " of truck-data.csv file." << '\n';
                cout << correct_truck_data;
                return -1;
            }
        }

        truck_file_contents.push_back(row); // Add the row vector to the truck data vector
    }

    cout << "Truck data has been successfully read. \n";
    truck_data.close();

    /* Open the data file containing the parcel information. */
    ifstream parcel_data("parcel-data.csv");
    if (!parcel_data.is_open())
    {
        cout << "Error opening parcel data file!";
        return -1;
    }

    /* Read csv file containing parcel data row by row and split each row by comma delimeter. */
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
            if (arg_counter > 4) // Check that a line in the data file has the correct number of entries
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
                    parcel_row.push_back(entry); // Add the comma separated value to the row vector
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
                    parcel_row.push_back(city); // Add the comma separated value to the row vector
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
        parcel_file_contents.push_back(parcel_row); // Add the row vector to the parcel data vector
    }

    cout << "Parcel data has been successfully read. \n";
    parcel_data.close();

    /* Open the data file containing the map information. */
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
            if (arg_counter > 3) // Check that a line in the data file has the correct number of entries
                throw invalid_argument("Too many data entries!");
            
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
                    map_row.push_back(city); // Add the comma separated value to the row vector
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
                    map_row.push_back(entry); // Add the comma separated value to the row vector
                }
            }
            catch(const invalid_argument &ex)
            {
                cerr << "Invalid data entry:" << entry << " found on line " << line_number << " of the map-data.csv file. " << ex.what() << '\n';
                cout << correct_map_data;
                return -1;
            }

            catch (const out_of_range &ex) 
            {
                cerr << "Number out of range: " << entry << " found on line" << line_number << " of the map-data.csv file. " << ex.what() << '\n';
                cout << correct_map_data;
                return -1;
            }
        }
        map_file_contents.push_back(map_row); // Add the row vector to the map data vector
    }

    cout << "Map data has been successfully read. \n";
    map_data.close();
    
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
                throw invalid_argument("Trailing characters after number!");

            string city_1 = map_entry.at(0);
            string city_2 = map_entry.at(1);

            newMap.add_distance(city_1, city_2, distance); // Add the map entry into the distance map
        }

        catch (const invalid_argument &ex) 
        {
            cerr << "Invalid distance found: " << ex.what() << '\n';
            cout << correct_map_data;
            return -1;
        } 
        catch (const out_of_range &ex) 
        {
            cerr << "Distance number out of range: " << ex.what() << '\n';
            cout << correct_map_data;
            return -1;
        }
    }
    
    cout << "Created distance map for parcel delivery: \n";
    newMap.print_distance_map(); // Print the distance map

    /* Use the data from the truck-data.csv to create truck objects and add them to a fleet of trucks. */
    vector<trucks> list_of_trucks; // Store the trucks read from the file
    /* Make copies for each scheduling algorithm. */
    vector<trucks> list_of_trucks_random;
    vector<trucks> list_of_trucks_most;
    vector<trucks> list_of_trucks_short;
    /* Make sure all trucks have a unique ID. */
    vector<uint64_t> unique_truck;

    /* Create truck objects and add them to each list for respective scheduling algorithms. */
    for (const vector<uint64_t> &truck_data : truck_file_contents)
    {
        try
        {
            trucks newtruck(truck_data[0], truck_data[1], COMMON_DEPOT);
            list_of_trucks.push_back(newtruck);
            list_of_trucks_random.push_back(newtruck);
            list_of_trucks_most.push_back(newtruck);
            list_of_trucks_short.push_back(newtruck);
            unique_truck.push_back(newtruck.my_id());
        }
        catch(const exception &e)
        {
            cerr << e.what() << '\n';
            return -1;
        }
    }

    /* Make sure that all trucks have a unique ID. */
    for (const uint64_t &id : unique_truck)
    {
        if (count(unique_truck.begin(), unique_truck.end(), id) != 1)
        {
            cout << "The parcel ID must be unique! \n";
            return -1;
        }
    }

    /* Use the data from the parcel-data.csv to create parcel objects. */
    vector<parcels> list_of_parcels; // Store the parcels read from the file
    vector<uint64_t> unique_parcel; // Make sure all parcels have a unique ID
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
            list_of_parcels.push_back(newparcel); // Add the parcel to the storage container
            unique_parcel.push_back(newparcel.this_id());
        }

        catch (const invalid_argument &ex) 
        {
            cerr << "Invalid number: " << ex.what() << '\n';
            cout << correct_parcel_data;
            return -1;
        } 
        catch (const out_of_range &ex) 
        {
            cerr << "Invalid number: " << ex.what() << '\n';
            cout << correct_parcel_data;
            return -1;
        }
    }

    /* Make sure all the parcels have a unique ID. */
    for (const uint64_t &id : unique_parcel)
    {
        if (count(unique_parcel.begin(), unique_parcel.end(), id) != 1)
        {
            cout << "The parcel ID must be unique! \n";
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

    /* Run some scheduling experiments using the data that was read from the input files. */
    randomScheduler pack_random_parcels(list_of_parcels, list_of_trucks_random);
    vector<parcels> randomparcel_unpacked = pack_random_parcels.schedule();

    mostparcelScheduler pack_most_parcels(list_of_parcels, list_of_trucks_most);
    vector<parcels> mostparcel_unpacked = pack_most_parcels.schedule();

    shortrouteScheduler pack_short_parcels(list_of_parcels, list_of_trucks_short);
    vector<parcels> shortparcel_unpacked = pack_short_parcels.schedule();

    /* Add these trucks to the fleet for deliveries. */
    fleet randomfleet;
    fleet mostparcelfleet;
    fleet shortroutefleet;

    try
    {
        load_fleet(list_of_trucks_random, randomfleet);
        load_fleet(list_of_trucks_most, mostparcelfleet);
        load_fleet(list_of_trucks_short, shortroutefleet);
    }
    catch(const exception &e)
    {
        cerr << e.what() << '\n';
    }

    /* Write the statistics to the output file. */
    
    try
    {
        route_stats << "Scheduler" << ", " << "Free Volume in Used Trucks (cm^3)" << ", " << "Average Capacity Used (%)" << ", " << "Std Dev Average Capacity" << ", " << "Avg Distance (km)" << ", " << "Std Dev Average Distance" << "\n";
        route_stats << "Random Parcels" << ", " << randomfleet.free_vol_in_used_trucks() << ", " << randomfleet.avg_capacity_used() << ", " << "+-" << randomfleet.std_dev_capacity_used() << ", " << mostparcelfleet.avg_distance_travelled(newMap) << ", " << "+-" << randomfleet.std_dev_distance_travelled(newMap) << "\n";
        route_stats << "Most Parcels" << ", " << mostparcelfleet.free_vol_in_used_trucks() << ", " << mostparcelfleet.avg_capacity_used() << ", " << "+-" << mostparcelfleet.std_dev_capacity_used() << ", " << mostparcelfleet.avg_distance_travelled(newMap) << ", " << "+-" << mostparcelfleet.std_dev_distance_travelled(newMap) << "\n";
        route_stats << "Short Route" << ", " << shortroutefleet.free_vol_in_used_trucks() << ", " << shortroutefleet.avg_capacity_used() << ", " << "+-" << shortroutefleet.std_dev_capacity_used() << ", " << shortroutefleet.avg_distance_travelled(newMap) << ", " << "+-" << shortroutefleet.std_dev_distance_travelled(newMap) << "\n";
        
    }
    catch(const map_invalidation::map_error &e)
    {
        cerr << e.what() << '\n';
        return -1;
    }
    
    cout << "The scheduling algorithm that randomly packs parcels into trucks suggests using the following delivery routes: \n";
    randomfleet.print_fleet(); // Print out the fleet schedule for this scheduling algorithm
    cout << "The scheduling algorithm that prioritizes packing the most possible parcels suggests using the following delivery routes: \n";
    mostparcelfleet.print_fleet(); // Print out the fleet schedule for this scheduling algorithm
    cout << "The scheduling algorithm that prioritizes shortest routes suggests using the following delivery routes: \n";
    shortroutefleet.print_fleet(); // Print out the fleet schedule for this scheduling algorithm

    if (randomparcel_unpacked.size() == 0)
        cout << "Using the Random Parcel scheduling algorithm all parcels were packed onto trucks. \n";
    else
    {
        cout << "Using the Random Parcel scheduling algorithm the following parcels could not be packed onto trucks: ";
        for (const parcels &parcel : randomparcel_unpacked)
            cout << parcel.this_id() << ", ";
        cout << "\n";
    }

    if (mostparcel_unpacked.size() == 0)
        cout << "Using the Most Parcel scheduling algorithm all parcels were packed onto trucks. \n";
    else
    {
        cout << "Using the Most Parcel scheduling algorithm the following parcels could not be packed onto trucks: ";
        for (const parcels &parcel : mostparcel_unpacked)
            cout << parcel.this_id() << ", ";
        cout << "\n";
    }

    if (shortparcel_unpacked.size() == 0)
        cout << "Using the Short Route scheduling algorithm all parcels were packed onto trucks. \n";
    else
    {
        cout << "Using the Short Route scheduling algorithm the following parcels could not be packed onto trucks: ";
        for (const parcels &parcel : shortparcel_unpacked)
            cout << parcel.this_id() << ", ";
        cout << "\n";
    }

    route_stats.close();
    cout << "The route statistics have been written to the route-stats.csv file. Here you will find information on each scheduling algorithm regarding the free volume left in the packed trucks, the average capacity used of the loaded trucks, as well as the standard deviation. You will also find information about the average distance travelled by the loaded trucks, as well as the standard deviation. \n";
}
