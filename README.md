# FedEx Parcel Delivery Scheduler

This program schedules parcels for delivery by assigning parcels to trucks and determining the route that each truck will take. Multiple scheduling options are offered so that a decision can be made about which route to take, with well packed trucks and short, efficient routes.

The program takes three files, one containing map data, one containing parcel data, and one containing truck data. The corresponding files must be named `map-data.csv`, `parcel-data.csv`, and `truck-data.csv` respectively. 

Every parcel must have an ID, source city, destination city, and a volume in cubic centimeters. This must be entered into the parcel file in this exact order with values separated by commas and each parcel on a new line. A parcels ID is unique, that is, no two parcels can have the same ID. An example of a `parcel-data.csv` file is as follows.


| 18 | London | Hamilton | 19 |
|:---|:---|:---|:---|
| 15| Belleville | Ottawa | 16 |
|14 | Ottawa | Guelph | 20|
|6| Belleville| Ottawa| 12|
|1| Guelph| Hamilton| 10|
|0| Toronto| Hamilton| 5|
|7| London| Guelph| 8|
|8| Ottawa| London| 21|
|12| London| Hamilton| 24|
|16| London| Guelph| 22|
|5| Ottawa| London| 21|
|11| Toronto| Hamilton| 8|
|3| Guelph| Hamilton| 20|
|9| London| Belleville| 7|
|13| Guelph| Hamilton| 15|


Every truck also has a unique ID. Trucks also have a maximum capacity in cubic centimeters, and as you load parcels into the truck the total volume of all parcels added cannot exceed the maximum capacity. A truck will also have a route that it will take to deliver the parcels that get loaded. An empty truck has an ID and maximum capacity. This must be entered into the truck file in this exact order with values separated by commas and each truck on a new line. An example of a `truck-data.csv` file is as follows.

|3| 35|
|:---|:---|
|8| 49|
|5| 33|
|6| 27|
|0| 42|


All trucks and all parcels will start from a common location called the depot. This depot must be a city in the `map-data.csv` file and there must de distance measures between the depot and all other relevant cities in th map. The depot is set by the user as an input argument. For example, if you want to run the program with the depot set to Toronto you would run `./main Toronto`.

A map is constructed from the map data file using the class `distanceMap`. Each entry in this file must contain two cities and the respective distance between them in kilometers. There must be a map entry for every city that a parcel must be delivered to, and this city must be connected to all other cities. An example of a `map-data.csv` file is as follows.

Belleville| Guelph| 265
|:---|:---|:---|
Belleville| Hamilton| 256
Belleville| Toronto| 187
Belleville| Ottawa| 269
Belleville| Kingston| 85
Belleville| London| 366
Belleville| Mississauga| 209
Belleville| Oakville| 223
Belleville| Windsor| 543
Belleville| Woodstock| 314
Guelph| Hamilton| 53
Guelph| Toronto| 93
Guelph| Woodstock| 67
Guelph| Ottawa| 528
Guelph| Kingston| 344
Guelph| London| 120
Guelph| Mississauga| 71
Guelph| Oakville| 70
Guelph| Windsor| 297
Hamilton| Toronto| 69
Hamilton| Woodstock| 79
Hamilton| Ottawa| 517
Hamilton| Kingston| 354
Hamilton| London| 128
Hamilton| Mississauga| 48
Hamilton| Oakville| 33
Hamilton| Windsor| 306
Toronto| Woodstock| 142
Toronto| Ottawa| 449
Toronto| Kingston| 242
Toronto| London| 192
Toronto| Mississauga| 28
Toronto| Oakville| 38
Toronto| Windsor| 369
Woodstock| Ottawa| 576
Woodstock| Kingston| 391
Woodstock| London| 55
Woodstock| Mississauga| 118
Woodstock| Oakville| 118
Woodstock| Windsor| 232
Ottawa| Kingston| 196
Ottawa| London| 627
Ottawa| Mississauga| 470
Ottawa| Oakville| 485
Ottawa| Windsor| 804
Kingston| London| 441
Kingston| Mississauga| 284
Kingston| Oakville| 298
Kingston| Windsor| 618
London| Mississauga| 172
London| Oakville| 157
London| Windsor| 192
Mississauga| Oakville| 22
Mississauga| Windsor| 349
Oakville| Windsor| 333

In this program the file `domain.hpp` defines the classes necessary to represent the parcels, trucks, and fleet of trucks. These classes are `parcels`, `trucks`, and `fleet`. A fleet keeps track of the trucks and also can report on statistics about the trucks such as average distance travelled and average capacity used of all the trucks in this fleet.

The file `schedule.hpp` defines three different scheduling algorithms to be implemented. These algorithms take the parcels and trucks that a user uploads and then sorts them into priority queues to be used for loading parcels onto trucks. The three different scheduling algorithms are implemented as follows: 

1. The random scheduler `randomScheduler` implements a scheduling algorithm that will load parcels onto trucks by randomly picking a truck to load a given parcel onto until all parcels have been loaded.
2. The `mostparcelScheduler` implements a scheduling algorithm that will order the parcels to be loaded onto trucks in a priority sequence where parcels with smaller volumes are loaded first. This priority queue of parcels is loaded onto trucks one by one. A truck is chosen based on the priority queue of available trucks. The potential trucks are ordered based on largest capacity, and then we take a subset of of these trucks who have enough available space to fit the parcel. Priority is given to trucks who have the parcel destination city already in their route. This algorithm generates a route that packs the most parcels onto the least trucks by prioritizing smaller parcels and larger trucks.
3. The `shortrouteScheduler` implements a scheduling algorithm that will order the parcels to be loaded onto trucks in a priority sequence that loads the parcels with the same destination sequentially. This priority queue of parcels is loaded onto trucks one by one. A truck is chosen based on the priority queue of available trucks. The potential trucks are ordered based on largest capacity, and then we take a subset of this list of trucks who have enough available space to fit the parcel. Priority is given to trucks who have the parcel destination city already in their route. This algorithm generates a route that packs trucks with parcels all headed to the same destination. This will result in shorter routes.

The program runs these various scheduling algorithms for the given parcels and trucks and outputs performance statistics regarding the average and standard deviation for free volume in loaded trucks, the average and standard deviation for the capacity used in loaded trucks, and the average and standard deviation for the distance travelled for loaded trucks in this fleet. An example of the performance statistics written to the `route-stats.csv` file for the input data described above is as follows:

Scheduler| Free Volume in Used Trucks (cm^3)| Average Capacity Used (%)| Std Dev Average Capacity| Avg Distance (km)| Std Dev Distance
|:---|:---|:---|:---|:---|:---|
Random Parcels| 25| 84.5357| +-12.5665| 536.8| +-132.59
Most Parcels| 46| 75.3034| +-13.8122| 536.8| +-396.749
Short Route| 16| 91.4323| +-6.7635| 275| +-224.113
