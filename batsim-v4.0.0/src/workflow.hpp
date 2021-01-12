/**
 * @file workflow.hpp
 * @brief Contains workflow-related classes
 */

#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <map>
#include <memory>

#include "pointers.hpp"
#include "pugixml.hpp"

struct Job;
class Task;

/**
 * @brief A workflow is a DAG of tasks, with points to
 *        source tasks and sink tasks
 */
class Workflow
{
public:
    /**
     * @brief Builds an empty Workflow
     * @param[in] name The Workflow name
     */
    explicit Workflow(const std::string & name);

    /**
     * @brief Workflow cannot be copied.
     * @param[in] other Another instance
     */
    Workflow(const Workflow & other) = delete;

    /**
     * @brief Destroys a Workflow
     */
    ~Workflow();

    /**
     * @brief Loads a complete workflow from an XML filename
     * @param[in] xml_filename The name of the XML file
     */
    void load_from_xml(const std::string & xml_filename);

    /**
     * @brief Checks whether a Workflow is valid (not needed since loading from XML?)
     */
    void check_validity();

    /**
     * @brief Adds a task to the workflow
     * @param[in] task The task to add into the Workflow
     */
    void add_task(Task &task);

    /**
     * @brief Gets a task based on its ID
     * @param[in] id The task id
     * @return The task corresponding to the given id
     */
    Task *get_task(std::string id);

    /**
     * @brief Add an edge between a parent task and a child task
     * @param[in,out] parent The parent task
     * @param[in,out] child The child task
     */
    void add_edge(Task &parent, Task &child);

    /**
     * @brief Gets source tasks
     * @return The source tasks
     */
    std::vector<Task *> get_source_tasks();

    /**
     * @brief Gets the sink tasks
     * @return The sink tasks
     */
    std::vector<Task *> get_sink_tasks();

    /**
     * @brief Gets the maximum depth
     * @return The maximum depth
     */
    int get_maximum_depth();

public:
    std::string filename;  //!< The DAX filename
    std::string name; //!< The Workflow name
    std::map<std::string, Task *> tasks; //!< Hashmap of all tasks
    double start_time = -1; //!< Workflow start time

private:
    pugi::xml_document dax_tree; //!< The DAX tree
};

/**
 * @brief A workflow Task is some attributes, pointers to parent tasks,
 *        and pointers to children tasks.
 */
class Task
{
public:
    /**
     * @brief Constructor
     * @param[in] num_procs The number of processors needed for the task
     * @param[in] execution_time The execution time of the task
     * @param[in] id The task id
     */
    Task(const int num_procs, const double execution_time, const std::string & id);

    /**
     * @brief Task cannot be copied.
     * @param[in] other Another instance
     */
    Task(const Task & other) = delete;

    /**
     * @brief Destructor
     */
    ~Task();

    /**
     * @brief Associates a Batsim Job to the task
     * @param[in] batsim_job The Batsim Job
     */
    void set_batsim_job(JobPtr batsim_job);


public:
    int num_procs; //!< The number of processors needed for the tas
    double execution_time; //!< The execution time of the task
    std::string id; //!< The task id
    JobPtr batsim_job = nullptr; //!< The batsim job created for this task
    std::vector<Task *> parents; //!< The parent
    std::vector<Task *> children; //!< The children
    int nb_parent_completed = 0; //!< The number of preceding tasks completed
    int depth = 0; //!< The task's top level
};


/**
 * @brief Handles a set of Workflows, identified by their names
 */
class Workflows
{
public:
    /**
     * @brief Builds an empty Workflows
     */
    Workflows();

    /**
     * @brief Workflows cannot be copied.
     * @param[in] other Another instance
     */
    Workflows(const Workflows & other) = delete;

    /**
     * @brief Destroys a Workflows
     */
    ~Workflows();

    /**
     * @brief Allows to access a Workflow thanks to its name
     * @param[in] workflow_name The name of the workflow to access
     * @return The workflow associated with the given workflow name
     * @pre The workflow exists
     */
    Workflow * operator[](const std::string & workflow_name);

    /**
     * @brief Allows to access a Workflow thanks to its name
     * @param[in] workflow_name The name of the workflow to access
     * @return The workflow associated with the given workflow name
     * @pre The workflow exists
     */
    const Workflow * operator[](const std::string & workflow_name) const;

    /**
     * @brief Allows to access a Workflow thanks to its name
     * @param[in] workflow_name The name of the workflow to access
     * @return The workflow associated with the given workflow name
     * @pre The workflow exists
     */
    Workflow * at(const std::string & workflow_name);

    /**
     * @brief Allows to access a Workflow thanks to its name
     * @param[in] workflow_name The name of the workflow to access
     * @return The workflow associated with the given workflow name
     * @pre The workflow exists
     */
    const Workflow * at(const std::string & workflow_name) const;

    /**
     * @brief Inserts a new Workflow into a Workflows
     * @param[in] workflow_name The name of the new Workflow to insert
     * @param[in] workflow The Workflow to insert
     * @pre There should be no existing Workflow with the same name in the Workflows
     */
    void insert_workflow(const std::string & workflow_name,
                         Workflow * workflow);

    /**
     * @brief Checks whether a Workflow with the given name exist.
     * @param[in] workflow_name The name of the Workflow whose existence is checked
     * @return true if a Workflow with the given name exists in the Workflows, false otherwise.
     */
    bool exists(const std::string & workflow_name) const;

    /**
     * @brief Returns the number of workflows
     * @return the number of workflows
     */
    bool size() const;


    /**
     * @brief Gets the internal map
     * @return The internal map
     */
     std::map<std::string, Workflow*> & workflows();

    /**
     * @brief Gets the internal map (const version)
     * @return The internal map (const version)
     */
      const std::map<std::string, Workflow*> & workflows() const;

private:
     std::map<std::string, Workflow*> _workflows; //!< Associates Workflows with their names
};

