/**
 * @file schedule.hpp
 * @author Cassandra Masschelein
 * @brief Define the different scheduling algorithms to be used to create parcel delivery schedules
 * @version 0.1
 * @date 2021-12-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/* C++ Header Files */
#pragma once
#include "domain.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>

using namespace std;

/**
 * @brief Find the trucks that have enough room to pack a parcel
 * 
 * @param p_volume The volume of the parcel
 * @param truck_list The list of potential trucks
 * @return The subset of trucks that have enough room to pack the parcel
 */
vector<trucks> enough_space(const uint64_t &p_volume, vector<trucks> &truck_list)
{
    vector<trucks> has_space;
    for (const trucks &truck : truck_list)
    {
        if (truck.avail_space >= p_volume)
            has_space.push_back(truck);
    }
    return has_space;
}

/**
 * @brief Check if truck A has a larger capacity than truck B
 * 
 * @param a The first truck
 * @param b The second truck
 * @return If it is true that truck A is larger than truck B
 */
bool larger_volume_truck(const trucks &a, const trucks &b)
{
    return a.volume() > b.volume();
}

/**
 * @brief Check if parcel A has a smaller volume than parcel B
 * 
 * @param a The first parcel
 * @param b The second parcel
 * @return If it is true that parcel A is smaller than parcel B
 */
bool smaller_volume_parcel(const parcels &a, const parcels &b)
{
    return a.volume() < b.volume();
}

/**
 * @brief Check if parcel A has a smaller destination than parcel B (alphabetically)
 * 
 * @param a Parcel A destination
 * @param b Parcel B destination
 * @return If it is true that the destination of parcel A is less than the destination of parcel B
 */
bool smaller_destination_parcel(const parcels &a, const parcels &b)
{
    return a.where_to() < b.where_to();
}

/**
 * @brief Check is the parcel destination is already in the trucks route
 * 
 * @param potential_trucks A list of trucks that could deliver the parcel
 * @param p The parcel to be packed and delivered
 * @return Either the same list of trucks or a subset of the list who have the destination already on route
 */
vector<trucks> on_route(vector<trucks> &potential_trucks, const parcels &p)
{
    /**
     * @brief A list of trucks that have the parcel destination already on route
     * 
     */
    vector<trucks> route_trucks;
    for (const trucks &truck : potential_trucks)
    {
        for (const string &stop : truck.route)
        {
            if (stop == p.where_to()) // Check if the parcels destination is already on route
            {
                route_trucks.push_back(truck);
                break;
            }
        }
    }
    if (route_trucks.size() != 0) // If no trucks already had the parcels destination on route, return the original list
        return route_trucks;
    return potential_trucks;
}

/**
 * @brief A random scheduler that determines what parcels go on which trucks, and what route the trucks will take
 * 
 */
class randomScheduler
{
public:
    /**
     * @brief Construct a new random Scheduler object
     * 
     * @param _parcel_list The list of parcels to be loaded on trucks and delivered
     * @param _truck_list The list of trucks available for delivering parcels
     */
    randomScheduler(const vector<parcels> &_parcel_list, vector<trucks> &_truck_list) : truck_list(_truck_list), parcel_list(_parcel_list) {}

    /**
     * @brief Schedule the given parcels onto the given trucks. Mutate truck objects but NOT parcel objects
     * 
     * @return A list of parcels that could not get loaded on trucks due to lack of capacity
     */
    vector<parcels> schedule()
    {
        for (const parcels &parcel : parcel_list)
            parcel_queue.push_back(parcel);
        
        /**
         * @brief A list of parcels that could not be loaded onto a truck for delivery
         * 
         */
        vector<parcels> not_packed_parcels;

        /* Load the parcels onto random trucks. */
        while (not parcel_queue.empty())
        {   
            /**
             * @brief The parcel to be loaded onto a truck
             * 
             */
            parcels parcel = parcel_queue[parcel_queue.size() - 1];
            parcel_queue.pop_back(); // Remove the last item in the parcel queue

            /* Create a priority queue of trucks based on increasing volume. */
            /**
             * @brief A list of trucks this parcel will fit on
             * 
             */
            vector<trucks> truck_candidates;
            for (const trucks &truck : truck_list)
            {
                if (truck.avail_space >= parcel.volume())
                    truck_candidates.push_back(truck);
            }

            if (truck_candidates.empty())
                not_packed_parcels.push_back(parcel); // We are unable to deliver the parcel
            else
            {
                uint64_t avail_length = truck_candidates.size();
                random_device rd;
                mt19937 mt(rd());
                uniform_int_distribution<uint64_t> uid(0, avail_length - 1);
                /**
                 * @brief A randomly choosen truck to load the parcel onto
                 * 
                 */
                trucks load_truck = truck_candidates[uid(mt)];

                uint64_t index = 0;
                for (trucks &truck : truck_list)
                {
                    if (truck.my_id() == load_truck.my_id())
                    {
                        truck_list[index].pack_truck(parcel);
                        break;
                    }
                    index += 1;
                }
            }
        }
        return not_packed_parcels;
    }
private:
    /**
     * @brief The list of trucks that are available for delivering parcels
     * 
     */
    vector<trucks> &truck_list;
    /**
     * @brief The list of parcels that need to be delivered
     * 
     */
    const vector<parcels> &parcel_list;
    /**
     * @brief The list of parcels to be packed onto trucks in priority sequence
     * 
     */
    vector<parcels> parcel_queue;
};

/**
 * @brief A small package large truck scheduler that determines what parcels go on which trucks, and what route the trucks wil take
 * 
 */
class mostparcelScheduler
{
public:
    /**
     * @brief Construct a new mostparcel Scheduler object
     * 
     * @param _parcel_list The list of parcels to be loaded on trucks and delivered
     * @param _truck_list The list of trucks available for delivering parcels
     */
    mostparcelScheduler(const vector<parcels> &_parcel_list, vector<trucks> &_truck_list) : truck_list(_truck_list), parcel_list(_parcel_list) {}

    /**
     * @brief Schedule parcels on trucks, priority given to smaller parcels and larger trucks to pack most parcels and use least trucks
     * 
     * @return A list of parcels that could not get loaded on trucks due to lack of capacity
     */
    vector<parcels> schedule()
    {
        /* Add the parcels to the parcel queue in priority sequence. */
        for (const parcels &parcel : parcel_list)
        {
            /* Check if the parcel queue has items and the given parcel has a lower priority than the last parcel in the queue. */
            if (not parcel_queue.empty() and not smaller_volume_parcel(parcel, parcel_queue.at(parcel_queue.size() - 1)))
            {
                uint64_t i = parcel_queue.size();
                /* Find where in the priority queue the parcel should be inserted. */
                while (i > 0 and not smaller_volume_parcel(parcel, parcel_queue[i - 1]))
                    i -= 1;
                parcel_queue.insert(parcel_queue.begin() + (int64_t)i, parcel);
            }
            else // The queue is empty so add the parcel to the queue
                parcel_queue.push_back(parcel);
        }

        /**
         * @brief A list of parcels that could not be loaded onto a truck for delivery
         * 
         */
        vector<parcels> not_packed_parcels;
        /* Load the parcels onto the trucks in priority sequence. */
        while (not parcel_queue.empty())
        {   
            truck_queue.clear(); // Re-initialize the queue for this parcel
            parcels parcel = parcel_queue[parcel_queue.size() - 1];
            parcel_queue.pop_back(); // Remove the last item in the parcel queue

            /**
             * @brief A list of trucks this parcel will fit on to be sorted
             * 
             */
            vector<trucks> truck_candidates;
            /* Create a priority queue of trucks based on increasing volume. */
            for (const trucks &truck : truck_list)
            {
                if (truck.avail_space >= parcel.volume())
                    truck_candidates.push_back(truck);
            }

            if (truck_candidates.empty())
                not_packed_parcels.push_back(parcel); // We are unable to deliver the parcel
            else
            {
                /**
                 * @brief Generate a list of trucks that already have the parcel destination on their routes, or use the original list if no trucks are on route
                 * 
                 */
                vector<trucks> checked_route = on_route(truck_candidates, parcel);
                for (const trucks &truck : checked_route)
                {
                    if (not truck_queue.empty() and not larger_volume_truck(truck, truck_queue.at(truck_queue.size() - 1)))
                    {
                        /* Check if the truck queue has items and the given truck has a lower priority than the last truck in the queue. */
                        uint64_t i = truck_queue.size();
                        /* Find where in the priority queue the truck should be inserted. */
                        while (i > 0 and not larger_volume_truck(truck, truck_queue[i - 1]))
                            i -= 1;
                        truck_queue.insert(truck_queue.begin() + (int64_t)i, truck);
                    }
                    else // The queue is empty so add the truck to the queue
                        truck_queue.push_back(truck);
                }
                /* Pack the parcel in the highest priority truck (the last truck in the truck queue). */
                /**
                 * @brief The truck with the highest priority
                 * 
                 */
                trucks load_truck = truck_queue.at(truck_queue.size() - 1); 
                uint64_t index = 0;
                /* Pack the highest priority truck. */
                for (trucks &truck : truck_list)
                {
                    if (truck.my_id() == load_truck.my_id())
                    {
                        truck_list[index].pack_truck(parcel);
                        break;
                    }
                    index += 1;
                }
            }
        }
        return not_packed_parcels;
    }
private:
/**
 * @brief The list of trucks that are available to pack
 * 
 */
    vector<trucks> &truck_list;
    /**
     * @brief The list of parcels to be packed onto trucks
     * 
     */
    const vector<parcels> &parcel_list;
    /**
     * @brief The list of trucks available to pack in priority sequence
     * 
     */
    vector<trucks> truck_queue;
    /**
     * @brief The list of parcels to be packed onto trucks in priority sequence
     * 
     */
    vector<parcels> parcel_queue;
};

/**
 * @brief A small parcel destination large truck scheduler that determines what parcels go on which trucks, and what route the trucks wil take
 * 
 */
class shortrouteScheduler
{
public:
    /**
     * @brief Construct a new shortroute Scheduler object
     * 
     * @param _parcel_list The list of parcels to be loaded on trucks and delivered
     * @param _truck_list The list of trucks available for delivering parcels
     */
    shortrouteScheduler(const vector<parcels> &_parcel_list, vector<trucks> &_truck_list) : truck_list(_truck_list), parcel_list(_parcel_list) {}

    /**
     * @brief Schedule parcels on trucks, priority given to parcels with smaller destinations and larger trucks to put parcels with same destination on same trucks
     * 
     * @return A list of parcels that could not get loaded on trucks due to lack of capacity
     */
    vector<parcels> schedule()
    {
        /* Add the parcels to the parcel queue in priority sequence. */
        for (const parcels &parcel : parcel_list)
        {
            /* Check if the parcel queue has items and the given parcel has a lower priority than the last parcel in the queue. */
            if (not parcel_queue.empty() and not smaller_destination_parcel(parcel, parcel_queue.at(parcel_queue.size() - 1)))
            {
                uint64_t i = parcel_queue.size();
                /* Find where in the priority queue the parcel should be inserted. */
                while (i > 0 and not smaller_destination_parcel(parcel, parcel_queue[i - 1]))
                    i -= 1;
                parcel_queue.insert(parcel_queue.begin() + (int64_t)i, parcel);
            }
            else // The queue is empty so add the parcel to the queue
                parcel_queue.push_back(parcel);
        }

        /**
         * @brief A list of parcels that could not be loaded onto a truck for delivery
         * 
         */
        vector<parcels> not_packed_parcels;
        /* Load the parcels onto the trucks in priority sequence. */
        while (not parcel_queue.empty())
        {   
            truck_queue.clear();
            parcels parcel = parcel_queue[parcel_queue.size() - 1];
            parcel_queue.pop_back(); // remove the last item in the parcel queue

            /**
             * @brief A list of trucks this parcel will fit on to be sorted
             * 
             */
            vector<trucks> truck_candidates;
            /* Create a priority queue of trucks based on increasing volume. */
            for (const trucks &truck : truck_list)
            {
                if (truck.avail_space >= parcel.volume())
                    truck_candidates.push_back(truck);
            }
            if (truck_candidates.empty())
                not_packed_parcels.push_back(parcel); // We are unable to deliver the parcel
            else
            {
                /**
                 * @brief Generate a list of trucks that already have the parcel destination on their routes, or use the original list if no trucks are on route
                 * 
                 */
                vector<trucks> checked_route = on_route(truck_candidates, parcel); // A list of trucks that already have the parcel destination on their route
                for (const trucks &truck : checked_route)
                {
                    if (not truck_queue.empty() and not larger_volume_truck(truck, truck_queue.at(truck_queue.size() - 1)))
                    {
                        /* Check if the truck queue has items and the given truck has a lower priority than the last truck in the queue. */
                        uint64_t i = truck_queue.size();
                        /* Find where in the priority queue the truck should be inserted. */
                        while (i > 0 and not larger_volume_truck(truck, truck_queue[i - 1]))
                            i -= 1;
                        truck_queue.insert(truck_queue.begin() + (int64_t)i, truck);
                    }
                    else // The queue is empty so add the truck to the queue
                        truck_queue.push_back(truck);
                }
                /* Pack the parcel in the highest priority truck (the last truck in the truck queue). */
                /**
                 * @brief The truck with the highest priority
                 * 
                 */
                trucks load_truck = truck_queue.at(truck_queue.size() - 1); 
                uint64_t index = 0;

                for (trucks &truck : truck_list)
                {
                    if (truck.my_id() == load_truck.my_id())
                    {
                        truck_list[index].pack_truck(parcel);
                        break;
                    }
                    index += 1;
                }
            }
        }
        return not_packed_parcels;
    }

private:
    /**
     * @brief The list of trucks that are available to pack
     * 
     */
    vector<trucks> &truck_list;
    /**
     * @brief The list of parcels to be packed onto trucks
     * 
     */
    const vector<parcels> &parcel_list;
    /**
     * @brief The list of trucks available to pack in priority sequence
     * 
     */
    vector<trucks> truck_queue;
    /**
     * @brief The list of parcels to be packed onto trucks in priority sequence
     * 
     */
    vector<parcels> parcel_queue;
};