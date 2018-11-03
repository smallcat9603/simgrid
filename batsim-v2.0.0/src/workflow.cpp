/**
 * @file workflow.cpp
 * @brief Contains workflow-related functions
 */

#include "workflow.hpp"

#include <fstream>
#include <streambuf>

#include "pugixml-1.7/pugixml.hpp"

#include "context.hpp"
#include "jobs.hpp"
#include "profiles.hpp"
#include "jobs_execution.hpp"

using namespace std;
using namespace pugi;

XBT_LOG_NEW_DEFAULT_CATEGORY(workflow, "workflow"); //!< Logging

Workflow::Workflow(const std::string & name) :
    name(name)
{
}

Workflow::~Workflow()
{
    // delete name;   TOFIX
    // name = nullptr;
    tasks.clear();

}

void Workflow::load_from_xml(const std::string &xml_filename)
{
    XBT_INFO("Loading XML workflow '%s'...", xml_filename.c_str());

    // XML document creation
    xml_parse_result result = dax_tree.load_file(xml_filename.c_str());
    (void) result; // Avoids a warning if assertions are ignored
    xbt_assert(result, "Invalid XML file");

    xml_node dag = dax_tree.child("adag");
    for (xml_node job = dag.child("job"); job; job = job.next_sibling("job"))
    {
        // Parse the number of processors, if any
        int num_procs = 1;
        if (job.attribute("num_procs"))
        {
            num_procs = (int)strtol(job.attribute("num_procs").value(),NULL,10);
        }
        if (num_procs <= 0)
        {
            num_procs = 1;
        }

        Task *task = new Task (num_procs, strtod(job.attribute("runtime").value(),NULL),
                               job.attribute("id").value());
        add_task(*task);
    }

    for (xml_node edge_bottom = dag.child("child"); edge_bottom;
         edge_bottom = edge_bottom.next_sibling("child"))
    {
        Task *dest = get_task(edge_bottom.attribute("ref").value());

        for (xml_node edge_top = edge_bottom.child("parent"); edge_top;
             edge_top = edge_top.next_sibling("parent"))
        {
            Task *source = get_task(edge_top.attribute("ref").value());

            //std::cout << "Test : " << source->id << " --> " << dest->id << std::endl;
            add_edge(*source,*dest);
        }
    }

    /* Testing things
    std::cout << get_source_tasks().size() << std::endl;

    Task *stuff1 = get_task("ID00017");
    Task *stuff2 = get_task("ID00018");

    std::cout << stuff1->execution_time << std::endl;

    add_edge(*stuff1,*stuff2);

    std::cout << get_sink_tasks().size() << std::endl;
    */

    /*
    // Let's try to read the number of machines in the XML document
    xbt_assert(doc.HasMember("nb_res"), "Invalid XML file '%s': the 'nb_res' field is missing", xml_filename.c_str());
    const Value & nb_res_node = doc["nb_res"];
    xbt_assert(nb_res_node.IsInt(), "Invalid XML file '%s': the 'nb_res' field is not an integer", xml_filename.c_str());
    nb_machines = nb_res_node.GetInt();
    xbt_assert(nb_machines > 0, "Invalid XML file '%s': the value of the 'nb_res' field is invalid (%d)",
               xml_filename.c_str(), nb_machines);

    jobs->load_from_xml(doc, xml_filename);
    profiles->load_from_xml(doc, xml_filename);
    */
    XBT_INFO("XML workflow parsed sucessfully.");
    XBT_INFO("Checking workflow validity...");
    check_validity();
    XBT_INFO("Workflow seems to be valid.");

    this->filename = xml_filename;
}


void Workflow::check_validity()
{
    // Likely not needed, so it doesn't do anything for now
    return;
}

void Workflow::add_task(Task &task)
{
    this->tasks[task.id] = &task;
}

Task * Workflow::get_task(std::string id)
{
    xbt_assert(this->tasks.count(id) == 1,
               "Invalid Workflow::get_task call: id '%s' does not exist", id.c_str());
    return this->tasks[id];
}

void Workflow::add_edge(Task &parent, Task &child)
{
    if(std::find(child.parents.begin(), child.parents.end(), &parent) != child.parents.end())
    {
        // Edge already there, no hyperedge
    }
    else
    {
        child.parents.push_back(&parent);
    }

    if(std::find(parent.children.begin(), parent.children.end(), &parent) != parent.children.end())
    {
        // Edge already there, no hyperedge
    }
    else
    {
        parent.children.push_back(&child);
    }
}


std::vector<Task *> Workflow::get_source_tasks()
{
    std::vector<Task *> task_list;
    for(std::map<std::string, Task *>::iterator it = this->tasks.begin(); it != this->tasks.end(); ++it)
    {
        if ((it->second)->parents.empty())
        {
            task_list.push_back(it->second);
        }
    }
    return task_list;
}

std::vector<Task *> Workflow::get_sink_tasks()
{
    std::vector<Task *> task_list;
    for(std::map<std::string, Task *>::iterator it = this->tasks.begin(); it != this->tasks.end(); ++it)
    {
        if ((it->second)->children.empty())
        {
            task_list.push_back(it->second);
        }
    }
    return task_list;
}


int Workflow::get_maximum_depth()
{
    int max_depth = -1;
    std::vector<Task *> sinks = this->get_sink_tasks();
    for (std::vector<Task*>::iterator it = sinks.begin(); it != sinks.end(); ++it)
    {
        if ((max_depth == -1) || ((*it)->depth > max_depth))
        {
            max_depth = (*it)->depth;
        }
    }
    return max_depth;
}



Task::Task(const int num_procs, const double execution_time, const std::string &id) :
    num_procs(num_procs),
    execution_time(execution_time),
    id(id)
{
}

Task::~Task()
{
    delete &(this->id);
    this->parents.clear();
    this->children.clear();
}



void Task::set_batsim_job(Job * batsim_job)
{
    this->batsim_job = batsim_job;
}


Workflows::Workflows()
{

}

Workflows::~Workflows()
{
    for (auto mit : _workflows)
    {
        Workflow * workflow = mit.second;
        delete workflow;
    }
    _workflows.clear();
}

Workflow *Workflows::operator[](const std::string &workflow_name)
{
    return at(workflow_name);
}

const Workflow *Workflows::operator[](const std::string &workflow_name) const
{
    return at(workflow_name);
}

Workflow *Workflows::at(const std::string &workflow_name)
{
    xbt_assert(exists(workflow_name));
    //    xbt_assert(false, "The next line is bad and has been added to make Batsim compile without warning (required by Travis). Please fix it.");
    //    return nullptr;
    return _workflows.at(workflow_name);
}

const Workflow *Workflows::at(const std::string &workflow_name) const
{
    xbt_assert(exists(workflow_name));
    //xbt_assert(false, "The next line is bad and has been added to make Batsim compile without warning (required by Travis). Please fix it.");
    //return nullptr;
    return _workflows.at(workflow_name);
}

void Workflows::insert_workflow(const std::string &workflow_name, Workflow *workflow)
{
    xbt_assert(!exists(workflow_name));
    xbt_assert(!exists(workflow->name));

    workflow->name = workflow_name;
    _workflows[workflow_name] = workflow;
}

bool Workflows::exists(const std::string &workflow_name) const
{
    //xbt_assert(false, "The next line is bad and has been added to make Batsim compile without warning (required by Travis). Please fix it.");
    //return workflow_name == "mmh";
    return _workflows.count(workflow_name) == 1;
}

bool Workflows::size() const
{
    return _workflows.size();
}


std::map<std::string, Workflow *> &Workflows::workflows()
{
    return _workflows;
}

const std::map<std::string, Workflow *> &Workflows::workflows() const
{
    return _workflows;
}

