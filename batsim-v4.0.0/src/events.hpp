/**
 * @file events.hpp
 * @brief Contains events-related classes
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include <rapidjson/document.h>
#include <intervalset.hpp>

/**
 * @brief Contains the different types of event
 */
enum class EventType
{
     EVENT_MACHINE_AVAILABLE        //!< The machine becomes available
    ,EVENT_MACHINE_UNAVAILABLE      //!< The machine becomes unavailable
    ,EVENT_GENERIC                  //!< A generic event
};

/**
 * @brief Returns a std::string corresponding to a given EventType
 * @param[in] type The Event type
 * @return A std::string corresponding to a given EventType
 */
std::string event_type_to_string(const EventType & type);

/**
 * @brief Returns an EventType corresponding to a given std::string
 * @param type_str The std::string
 * @param unknown_as_generic Whether an unknown event has to be treated as a generic Event
 * @return An EventType corresponding to a given std::string
 */
EventType event_type_from_string(const std::string & type_str, const bool unknown_as_generic = false);

/**
 * @brief The data of an EVENT_MACHINE_AVAILABLE or EVENT_MACHINE_UNAVAILABLE
 */
struct MachineAvailabilityEventData
{
    IntervalSet machine_ids;    //!< The machine ids involved
};

/**
 * @brief The data of an EVENT_GENERIC
 */
struct GenericEventData
{
    std::string json_desc_str; //!< The json description of the generic event as a string
};

/**
 * @brief Represents an event
 */
struct Event
{
    Event() = default;
    ~Event();

    EventType type;             //!< The type of the Event
    long double timestamp = -1; //!< The occurring time of the Event
    void * data = nullptr;      //!< The Event data

public:
    /**
     * @brief Creates a new-allocated Event from a JSON description
     * @param[in] json_desc The JSON description of the event
     * @param[in] unknown_as_generic Whether an unknown event has to be treated as a generic Event
     * @param[in] error_prefix The prefix to display when an error occurs
     * @return The newly allocated Event
     */
    static Event * from_json(rapidjson::Value & json_desc,
                             const bool unknown_as_generic = false,
                             const std::string & error_prefix = "Invalid JSON event");

    /**
     * @brief Creates a new-allocated Event from a JSON description
     * @param[in] json_str The JSON description of the event (as a string)
     * @param[in] unknown_as_generic Whether an unknown event has to be treated as a generic Event
     * @param[in] error_prefix The prefix to display when an error occurs
     * @return The newly allocated Event
     */
    static Event * from_json(const std::string & json_str,
                             const bool unknown_as_generic = false,
                             const std::string & error_prefix = "Invalid JSON event");

};

/**
 * @brief Compares events thanks to their timestamps
 * @param a The first Event
 * @param b The second Event
 * @return  True if and only if the first event's timestamp is strictly lower than the second event's timestamp
 */
bool event_comparator_timestamp_number(const Event * a, const Event * b);

/**
 * @brief List of Events to be submitted via an event_submitter
 */
class EventList
{
public:
    /**
     * @brief Creates an empty EventList
     * @param[in] name The name of the EventList
     * @param[in] is_static Whether this EventList is static or not
     */
    explicit EventList(const std::string & name = "unset", const bool is_static = true);

    /**
      * @brief Destroys an EventList
      */
    ~EventList();

    /**
     * @brief Loads static Events from a JSON filename
     * @param[in] json_filename The name of the JSON file
     * @param[in] unknown_as_generic Whether an unknown event has to be treated as a generic Event
     */
    void load_from_json(const std::string & json_filename, bool unknown_as_generic = false);

    /**
     * @brief Gets the list of Events
     * @return The internal vector of Event*
     */
    std::vector<Event*> & events();

    /**
     * @brief Gets the list of Events (const version)
     * @return The internal vector of Event* (const version)
     */
    const std::vector<Event*> & events() const;

    /**
     * @brief Adds an event to the EventList
     * @param[in] event The Event to add to the list
     */
    void add_event(Event * event);

    /**
     * @brief Returns whether the EventList is static (corresponding to a Batsim input EventList) or not
     * @return  Whether the EventList is static or not
     */
    bool is_static() const;

private:
    std::vector<Event*> _events;    //!< The list of events (should be sorted in non-decreasing timestamp)
    std::string _name;              //!< The name of the event list
    std::string _file = "";         //!< The filename of the event list, if any
    bool _is_static;                //!< Whether the EventList is dynamic
};
