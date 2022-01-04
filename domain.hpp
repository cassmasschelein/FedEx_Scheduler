/**
 * @file domain.hpp
 * @author Cassandra Masschelein
 * @brief Define the classes necessary to represent the parcels, trucks, and fleet of trucks
 * @version 0.1
 * @date 2021-12-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/* C++ Header Files */
#pragma once
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <stdexcept>
#include <iomanip>

using namespace std;

/* This is the common depot where all trucks and parcels begin. */
const string COMMON_DEPOT = "Toronto";

namespace map_invalidation
{
    class map_error : public invalid_argument
    {
        public:
            map_error() : invalid_argument("These cities cannot be found in the Distance Map!"){};
    };

    class city_error : public invalid_argument
    {
        public:
            city_error() : invalid_argument("The source city and destination city must be different!"){};
    };
}

namespace parcel_invalidation
{
    class unique_id : public invalid_argument
    {
        public:
            unique_id() : invalid_argument("The parcel ID must be small and unique!"){};
    };
}

namespace truck_invalidation
{
    class unique_id : public invalid_argument
    {
        public:
            unique_id() : invalid_argument("The truck ID must be small and unique!"){};
    };

    class mismatch_depot : public invalid_argument
    {
        public:
            mismatch_depot() : invalid_argument("The truck has the wrong depot for this fleet!"){};
    };
}

/**
 * @brief A parcel that needs to be delivered. A parcel has an ID, a volume, a source city, and a destination city
 * 
 */
class parcels
{
    friend class trucks;

public:
    /* Constructor with arguments read from the input data file parcel_data.txt. Pre-condition: the parcel already arrived at the depot. */
    parcels(const uint64_t &this_id, const uint64_t &_vol, const string &_source_city, const string &_dest) : p_id(this_id), p_vol(_vol), source_city(_source_city), dest_city(_dest)
    {
        /* A parcels source city and destination city cannot be the same. */
        if (source_city == dest_city)
            throw map_invalidation::city_error();
        if (p_id > 100000) // A parcel ID must be unique
        {
            throw parcel_invalidation::unique_id();
        }
    }

    /**
     * @brief Return the volume of a parcel
     * 
     * @return volume in cm^3
     */
    uint64_t volume() const
    {
        return p_vol;
    }

    /**
     * @brief Return the city where this parcel is being sent
     * 
     * @return The destination city 
     */
    string where_to() const
    {
        return dest_city;
    }

private:
    /* Each parcel has an ID that is unique, volume, source city, and destination city where this parcel needs to go. */
    uint64_t p_id, p_vol;
    string source_city, dest_city;
};

/**
 * @brief A truck that will be used to deliver parcels. A truck has an id, a capacity, available space left on the truck, a depot where it starts from, a route, and a list of parcels
 * 
 */
class trucks
{
    friend class fleet;

public:
    /* Constructor with arguments read from the input data file truck_data.txt. A truck has a unique ID, capacity, available space, route, depot, and parcels. */
    trucks(const uint64_t &_id, const uint64_t &_cap) : avail_space(_cap), t_id(_id), t_cap(_cap), depot(COMMON_DEPOT) 
    {
        if (t_id > 100000) // The ID is not small and unique
        {
            throw truck_invalidation::unique_id();
        }

        if (depot != COMMON_DEPOT) // The truck is not starting at the depot
        {
            throw truck_invalidation::mismatch_depot();
        }

        route.push_back(depot); // Add the depot as the first stop on the route
        parcels_list = vector<uint64_t>(0); // A list of parcels on this truck
        total_trucks++;
        truck_number = total_trucks;
    }

    uint64_t avail_space; // Volume available in a truck to fill with parcels
    vector<string> route; // The route that a given truck will take
    vector<uint64_t> parcels_list; // The list of parcels (by id) that are loaded onto this truck

    /**
     * @brief Load a parcel onto a truck
     * 
     * @param parcel The parcel to potentially be loaded
     * @return True or False whether that parcel was loaded
     */
    bool pack_truck(const parcels &parcel)
    {
        if (parcel.p_vol <= avail_space) // If the parcel will fit on the truck, load it on the truck
        {
            parcels_list.push_back(parcel.p_id);
            avail_space -= parcel.p_vol;
            if (find(route.begin(), route.end(), parcel.dest_city) == route.end()) // If the parcel destination is not in the route, add it to the end of the route
                route.push_back(parcel.dest_city);
            return true;
        }
        return false;
    }

    /**
     * @brief The trucks capacity
     * 
     * @return The volume of the truck in cm^3 
     */
    uint64_t volume() const
    {
        return t_cap;
    }

    /**
     * @brief A function that returns the capacity that has been used for a given truck
     * 
     * @return The percentage of used space
     */
    double capacity_used() const
    {
        double percentage = (double)avail_space / (double)t_cap;
        return 100.0 - (percentage * 100.0);
    }

    /**
     * @brief Get the total number of trucks
     * 
     * @return The number of trucks that we have available to use
     */
    static uint64_t get_total_trucks()
    {
        return total_trucks;
    }

private:
    /* Each truck has an ID and capacity that is read from the input file trucks.txt. */
    uint64_t t_id;
    uint64_t t_cap;
    string depot; // The depot where this truck will be starting from
    inline static uint64_t total_trucks = 0; // Keep track of how many trucks we have
    uint64_t truck_number; // This trucks number
};

/**
 * @brief A map that stores the distances between cities for all cities where parcels must be delivered
 * 
 */
class distanceMap
{
    friend class fleet;

public:
    // The default constructor of this class with an empty map
    distanceMap() {}

    /**
     * @brief Add the distance between two cities to the distance map
     * 
     * @param city_1 The first city
     * @param city_2 The second city
     * @param distance The distance between the cities in km
     */
    void add_distance(const string &city_1, const string &city_2, const uint64_t &distance)
    {
        vector<string> this_tuple = {city_1, city_2};
        if (distance_map.count(this_tuple) == 0)
            distance_map[this_tuple] = distance;
    }

    /**
     * @brief Find the distance between two given cities (in km)
     * 
     * @param city_1 The source city
     * @param city_2 The destination city
     * @return The distance between the two cities (in km)
     */
    uint64_t distance(const string &city_1, const string &city_2) const
    {
        vector<string> this_tuple = {city_1, city_2};
        vector<string> reverse_tuple = {city_2, city_1};
        if (distance_map.count(this_tuple) == 1)
            return distance_map.at(this_tuple);
        else if (distance_map.count(reverse_tuple) == 1)
            return distance_map.at(reverse_tuple);
        else
            throw map_invalidation::map_error();
    }

    /**
     * @brief Print the distance map as a dictionary 
     * 
     */
    void print_distance_map() const
    {
        cout << "{ \n";
        for (const auto &i : distance_map)        
            cout << setw(5) << "(" << i.first[0] << ", " << i.first[1] << "): " << i.second << "\n";
        
        cout << "} \n";
    }

private:
    map<vector<string>, uint64_t> distance_map; // A map where each key is a tuple of the two cities and the value is the distance between them
};


/**
 * @brief A fleet of trucks for delivering parcels. A fleet will have a list of trucks.
 * 
 */
class fleet
{
public:
    // The default constructor for this class with no trucks
    fleet() {}

    /**
     * @brief A function for adding a truck to this fleet of trucks, and adding the map entry
     * 
     * @param f_trucks A truck object to be added to the fleet
     */
    void add_truck(const trucks &truck)
    {
        uint64_t counter = 0;
        /* Check if the truck has already been added to the fleet. */
        for (trucks &curr_truck : f_trucks)
        {
            if (curr_truck.t_id == truck.t_id) // Check by ID since IDs are unique
                counter += 1;
                // throw some kind of unique id error
        }
        if (counter == 0) // The truck has not already been added to the fleet
        {
            f_trucks.push_back(truck);
            parcel_alloc[truck.t_id] = truck.parcels_list;
        }
    }

    /**
     * @brief A function to print a written representation of this fleet
     * 
     */
    void print_fleet() const
    {
        for (const trucks &truck : f_trucks)
        {
            cout << "Truck: " << truck.t_id << " Route: ";
            for (const string &t_route : truck.route)
            {
                cout << t_route << " -> ";
            }
            cout << "\n";
        }
    }

    /**
     * @brief A function that returns the total number of trucks in this fleet
     * 
     * @return The total number of trucks
     */
    uint64_t number_of_trucks() const
    {
        // return f_trucks.size();
        return trucks::get_total_trucks();
    }

    /**
     * @brief A function to return the number of trucks with parcels on them
     * 
     * @return The number of trucks with loaded parcels
     */
    uint64_t number_trucks_used() const
    {
        uint64_t counter = 0;
        for (const trucks &truck : f_trucks)
        {
            if (truck.avail_space != truck.t_cap)
                counter += 1;
        }
        return counter;
    }

    /**
     * @brief Return the available volume in trucks with parcels
     * 
     * @return The volume of available space on loaded trucks
     */
    uint64_t free_vol_in_used_trucks() const
    {
        uint64_t volume = 0;
        for (const trucks &truck : f_trucks)
        {
            if (truck.avail_space != truck.t_cap) // Check for trucks that have parcels loaded on them
                volume += truck.avail_space;
        }
        return volume;
    }

    /**
     * @brief Return that average capacity used for all trucks with loaded parcels
     * 
     * @return The volume of the trucks capacity used as a percentage 
     */
    double avg_capacity_used() const
    {
        double avg_cap_ = 0.0;
        uint64_t N = number_trucks_used();
        if (N != 0)
        {
            double tot_cap = 0.0;
            for (const trucks &truck : f_trucks)
            {
                tot_cap += truck.capacity_used(); 
            }
            avg_cap_ = tot_cap / (double)N;
        }
        return avg_cap_;
    }

    /**
     * @brief Calculate the standard deviation for the capacity used of loaded trucks
     * 
     * @return The standard deviation of capacity used 
     */
    double std_dev_capacity_used() const
    {
        double std_dev = 0.0;
        uint64_t N = number_trucks_used();
        if (N != 0)
        {
            double mean_cap = avg_capacity_used();
            double sum_num_minus_mean = 0.0;
            double over_N = 1.0 / (double)N;
            for (const trucks &truck : f_trucks)
            {
                sum_num_minus_mean += pow(truck.capacity_used() - mean_cap, 2.0);
            }
            std_dev = sqrt(over_N * sum_num_minus_mean);
        }
        return std_dev;
    }

    /**
     * @brief Calculate the average distance travelled by trucks in this fleet
     * 
     * @return The average distance travelled  
     */
    double avg_distance_travelled(const distanceMap &dmap) const
    {
        double avg_distance = 0.0;
        uint64_t distance_travel = 0;
        uint64_t N = number_trucks_used();
        if (N != 0)
        {
            for (const trucks &truck : f_trucks)
            {
                for (uint64_t i = 0; i < truck.route.size(); i++)
                {
                    distance_travel += dmap.distance(truck.route[i], truck.route[i + 1]); 
                }
                if (truck.route.size() > 1)
                    distance_travel += dmap.distance(truck.route[0], truck.route[truck.route.size() - 1]);
            }
            avg_distance = (double)distance_travel / (double)N;
        }
        return avg_distance;
    }

    /**
     * @brief Calculate the standard deviation for the distance travelled by the trucks in this fleet
     * 
     * @return The standard deviations for distance travelled 
     */
    double std_dev_distance_travelled(const distanceMap &dmap) const
    {
        double std_dev = 0.0;
        uint64_t N = number_trucks_used();
        if (N != 0)
        {
            double mean_dist = avg_distance_travelled(dmap);
            double sum_num_minus_mean = 0.0;
            double over_N = 1.0 / (double)N;
            for (const trucks &truck : f_trucks)
            {
                uint64_t this_truck = 0;
                for (uint64_t i = 0; i < truck.route.size(); i++)
                {
                    this_truck += dmap.distance(truck.route[i], truck.route[i + 1]);
                }
                if (truck.route.size() > 1)
                    this_truck += dmap.distance(truck.route[0], truck.route[truck.route.size() - 1]);

                sum_num_minus_mean += pow((double)this_truck - mean_dist, 2.0);
            }
            std_dev = sqrt(over_N * sum_num_minus_mean);
        }
        return std_dev;
    }

private:
    vector<trucks> f_trucks; // The list of trucks in this fleet, stored in a vector
    map<uint64_t, vector<uint64_t> > parcel_alloc; // A map of truck IDs in the fleet and the parcel IDs loaded in loading order
};
