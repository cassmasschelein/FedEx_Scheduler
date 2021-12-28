/* C++ Header Files */
#pragma once
#include "domain.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>

using namespace std;

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

bool larger_volume_truck(const trucks &a, const trucks &b)
{
    return a.volume() > b.volume();
}

bool smaller_volume_parcel(const parcels &a, const parcels &b)
{
    return a.volume() < b.volume();
}

bool smaller_destination_parcel(const parcels &a, const parcels &b)
{
    return a.where_to() < b.where_to();
}

// take a list of trucks and check if the parcel destination is already on route. return the modified list of trucks if they are on route
vector<trucks> on_route(vector<trucks> &potential_trucks, const parcels &p)
{
    vector<trucks> route_trucks;
    for (const trucks &truck : potential_trucks)
    {
        for (const string &stop : truck.route)
        {
            if (stop == p.where_to())
            {
                route_trucks.push_back(truck);
                break;
            }
        }
    }
    if (route_trucks.size() != 0)
        return route_trucks;
    return potential_trucks;
}

// put all priority functions here

class theScheduler
{
public:
    theScheduler(const vector<parcels> &_parcel_list, vector<trucks> &_truck_list) : truck_list(_truck_list), parcel_list(_parcel_list) {}

    virtual vector<parcels> schedule()
    {
        vector<parcels> not_packed_parcels;

        for (const parcels &parcel : parcel_list)
        {
            vector<trucks> avail_trucks = enough_space(parcel.volume(), truck_list);
            uint64_t avail_length = avail_trucks.size();
            if (avail_length > 0)
            {
                random_device rd;
                mt19937 mt(rd());
                uniform_int_distribution<uint64_t> uid(0, avail_length - 1);
                // Select a random truck to pack the parcel on
                trucks truck = avail_trucks[uid(mt)];
                truck.pack_truck(parcel);
            }
            else
                not_packed_parcels.push_back(parcel);
        }
        return not_packed_parcels;
    }

private:
    vector<trucks> truck_list;
    vector<parcels> parcel_list;
};

// a scheduler that loads all the small packages into the largest trucks so as to use fewer trucks and load the most parcels
class mostparcelScheduler : public theScheduler
{
public:
    mostparcelScheduler(const vector<parcels> &_parcel_list, vector<trucks> &_truck_list) : theScheduler(_parcel_list, _truck_list) {}

    // a function that will schedule the parcels to be delivered on which trucks. Mutate the trucks but not the parcels
    virtual vector<parcels> schedule()
    {
        // add the parcels to the parcel queue in priority sequence
        for (const parcels &parcel : parcel_list)
        {
            if (not parcel_queue.empty() and not smaller_volume_parcel(parcel, parcel_queue.at(parcel_queue.size() - 1)))
            {
                int64_t i = parcel_queue.size() - 1;
                while (i >= 0 and not smaller_volume_parcel(parcel, parcel_queue[i]))
                    i -= 1;
                parcel_queue.insert(parcel_queue.begin() + i + 1, parcel);
            }
            else
                parcel_queue.push_back(parcel);
        }

        // load the parcels onto the trucks
        vector<parcels> not_packed_parcels;
        while (not parcel_queue.empty())
        {   
            parcels parcel = parcel_queue[parcel_queue.size() - 1];
            parcel_queue.pop_back(); // remove the last item in the parcel queue

            // create a priority queue of trucks based on increasing volume
            vector<trucks> truck_candidates; // a list of trucks this parcel will fit on to be sorted
            for (const trucks &truck : truck_list)
            {
                if (truck.avail_space >= parcel.volume())
                    truck_candidates.push_back(truck);
            }
            if (truck_candidates.empty())
                not_packed_parcels.push_back(parcel); // we are unable to deliver the parcel
            else
            {
                vector<trucks> checked_route = on_route(truck_candidates, parcel);
                for (const trucks &truck : checked_route)
                {
                    if (not truck_queue.empty() and not larger_volume_truck(truck, truck_queue.at(truck_queue.size() - 1)))
                    {
                        int64_t i = truck_queue.size() - 1;
                        while (i >= 0 and not larger_volume_truck(truck, truck_queue[i]))
                            i -= 1;
                        truck_queue.insert(truck_queue.begin() + i + 1, truck);
                    }
                    else
                        truck_queue.push_back(truck);
                }
                // pack the parcel in the highest priority truck
                trucks pack_truck = truck_queue.at(truck_queue.size() - 1);
                pack_truck.pack_truck(parcel);
            }
        }

        return not_packed_parcels;
    }

private:
    vector<trucks> truck_queue;
    vector<parcels> parcel_queue;
    vector<trucks> truck_list;
    vector<parcels> parcel_list;
};

class shortrouteScheduler : public theScheduler
{
public:
    shortrouteScheduler(const vector<parcels> &_parcel_list, vector<trucks> &_truck_list) : theScheduler(_parcel_list, _truck_list) {}

    virtual vector<parcels> schedule()
    {
        // add the parcels to the parcel queue in priority sequence
        for (const parcels &parcel : parcel_list)
        {
            if (not parcel_queue.empty() and not smaller_destination_parcel(parcel, parcel_queue.at(parcel_queue.size() - 1)))
            {
                int64_t i = parcel_queue.size() - 1;
                while (i >= 0 and not smaller_destination_parcel(parcel, parcel_queue[i]))
                    i -= 1;
                parcel_queue.insert(parcel_queue.begin() + i + 1, parcel);
            }
            else
                parcel_queue.push_back(parcel);
        }

        // load the parcels onto the trucks
        vector<parcels> not_packed_parcels;
        while (not parcel_queue.empty())
        {   
            parcels parcel = parcel_queue[parcel_queue.size() - 1];
            parcel_queue.pop_back(); // remove the last item in the parcel queue

            // create a priority queue of trucks based on increasing volume
            vector<trucks> truck_candidates; // a list of trucks this parcel will fit on to be sorted
            for (const trucks &truck : truck_list)
            {
                if (truck.avail_space >= parcel.volume())
                    truck_candidates.push_back(truck);
            }
            if (truck_candidates.empty())
                not_packed_parcels.push_back(parcel); // we are unable to deliver the parcel
            else
            {
                vector<trucks> checked_route = on_route(truck_candidates, parcel);
                for (const trucks &truck : checked_route)
                {
                    if (not truck_queue.empty() and not larger_volume_truck(truck, truck_queue.at(truck_queue.size() - 1)))
                    {
                        int64_t i = truck_queue.size() - 1;
                        while (i >= 0 and not larger_volume_truck(truck, truck_queue[i]))
                            i -= 1;
                        truck_queue.insert(truck_queue.begin() + i + 1, truck);
                    }
                    else
                        truck_queue.push_back(truck);
                }
                // pack the parcel in the highest priority truck
                trucks pack_truck = truck_queue.at(truck_queue.size() - 1);
                pack_truck.pack_truck(parcel);
            }
        }

        return not_packed_parcels;
    }

private:
    vector<trucks> truck_queue;
    vector<parcels> parcel_queue;
    vector<trucks> truck_list;
    vector<parcels> parcel_list;
};