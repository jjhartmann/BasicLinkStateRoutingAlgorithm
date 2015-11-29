#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <tuple>
#include <functional>
#include <set>
#include <map>
#include <queue>
#include <limits.h>

using namespace std;
typedef std::tuple<string, string, int> EdgeTuple;
typedef multimap<string, EdgeTuple> EdgeMap;

enum {
    SNODE = 0,
    ENODE,
    WEIGHT
};


class LinkState
{
public:
    LinkState(string filename)
    {
        // Parse the file for use in Dijkstra's Algorithm
        ifstream infile(filename);

        string line;
        getline(infile, line);

        m_nodesN = atoi(line.substr(0, 2).c_str());


        // Get the m_nodes
        getline(infile, line);
        for (int i = 0; i < m_nodesN; ++i)
        {
            m_nodes.push_back(line.substr(i * 2, 1));
        }


        // Create edges with weights.
        for (int i = 0; i < m_nodesN; ++i)
        {
            getline(infile, line);
            string sNode = m_nodes[i];

            int index = 0;
            int pindex = 0;
            for (int j = 0; j < m_nodesN; ++j)
            {
                index = line.find(" ", pindex);
                string eNode = m_nodes[j];

                int weight = atoi(line.substr(pindex, index - pindex).c_str());

                // THere are no loops or neg numbers, so just continue.
                if (eNode == sNode || weight < 0)
                {
                    pindex = index + 1;
                    continue;
                }

                auto tuple = make_tuple(ref(sNode), ref(eNode), weight);
                m_edges.insert(make_pair(sNode, tuple));

                pindex = index + 1;
            }
        }

        // Close the file.
        infile.close();

    }

    // Main Algorithm, Creates the routing table by finding the min
    // Distance from source node to other nodes in network.
    void CreateRoutingTable()
    {

        // Start with initial node.
        string node = *m_nodes.begin();
        int currWeight = 0;


        // Build map of least cost routing
        int index = 0;
        while (index++ < m_nodes.size() - 1)
        {
            // Build Edge List
            buildEdgeList(node, currWeight);

            // Add node to visited nodes
            m_vistedNodes.insert(node);

            // Find Min edge in list.
            EdgeTuple min_edge = findMinEdge();

            // Add the min edge to the routing map
            m_routingmap.insert(make_pair(get<SNODE>(min_edge), min_edge));
            currWeight = get<WEIGHT>(min_edge);
            node = get<ENODE>(min_edge);
        }


        // Build the Forwarding table.
        EdgeMap::iterator sourceNode = m_routingmap.find(*m_nodes.begin());
        vector<string>::iterator iter = m_nodes.begin();

        // Iterate through all nodes and build table
        while (iter != m_nodes.end())
        {
            if (*iter == sourceNode->first)
            {
                ++iter;
                continue;
            }

            // Iterate through possible source paths
            EdgeMap::iterator siter = sourceNode;
            while (siter != m_routingmap.end() &&
                   siter->first == sourceNode->first)
            {
                int cost = 0;
                if (findNode(siter, *iter, cost))
                {
                    EdgeTuple res = siter->second;
                    get<WEIGHT>(res) = cost;
                    m_forwardingTable.insert(make_pair(*iter, res));

                    // Set iter to end, and continue with next node.
                    siter = m_routingmap.end();
                }
                else
                {
                    ++siter;
                }
            }

            ++iter;
        }


    }

    // Print the forwarding table
    void Print()
    {
        EdgeMap::iterator iter = m_forwardingTable.begin();

        while (iter != m_forwardingTable.end())
        {
            cout << iter->first << " (" << get<SNODE>(iter->second)
                    << "-" << get<ENODE>(iter->second) << ") "
                    << get<WEIGHT>(iter->second) << endl;

            ++iter;
        }
    }

    // Pivate methods
private:

    // Determine if node is in current source edge
    // Return true if so.
    bool findNode(EdgeMap::iterator &snode, string currNode, int &cost)
    {
        if (get<ENODE>(snode->second) == currNode)
        {
            cost = get<WEIGHT>(snode->second);
            return  true;
        }

        queue<string> Q;
        Q.push(get<ENODE>(snode->second));

        while (!Q.empty())
        {
            string node = Q.front();
            EdgeMap::iterator outerIter = m_routingmap.find(node);
            Q.pop();

            while (outerIter != m_routingmap.end() &&
                   outerIter->first == node)
            {
                if (get<ENODE>(outerIter->second) == currNode)
                {
                    // Node Found return weigth/cost
                    cost = get<WEIGHT>(outerIter->second);
                    return true;
                }

                Q.push(get<ENODE>(outerIter->second));
                ++outerIter;
            }
        }

        // Nothing found in routing map.
        return false;
    }


    // Builds a list of edges with adjusted for all viewed nodes
    void buildEdgeList(const string node, const int wieght)
    {
        EdgeMap::iterator iter = m_edges.find(node);
        while (iter != m_edges.end() && iter->first == node)
        {
            EdgeTuple tpl = iter->second;
            get<WEIGHT>(tpl) += wieght;

            if (!duplicateEdge(tpl))
            {
                m_edgelist.insert(make_pair(get<SNODE>(tpl), tpl));
            }

            ++iter;
        }
    }

    // Determines if there is a duplicate edge in list
    // ie X->Y == Y->X
    bool duplicateEdge(EdgeTuple tuple)
    {
        string enode = get<ENODE>(tuple);
        string snode = get<SNODE>(tuple);
        EdgeMap::iterator iter = m_edgeused.find(enode);
        while (iter != m_edgeused.end()&& iter->first == enode)
        {
            if (get<ENODE>(iter->second) == snode)
            {
                return true;
            }

            ++iter;
        }

        return false;
    }

    // Finds the minimum weight in the list of edges.
    EdgeTuple findMinEdge()
    {
        EdgeMap::iterator iter = m_edgelist.begin();
        EdgeMap::iterator res;
        int currWeight = INT_MAX;
        while (iter != m_edgelist.end())
        {
            if (get<WEIGHT>(iter->second) < currWeight &&
                m_vistedNodes.find(get<ENODE>(iter->second)) == m_vistedNodes.end())
            {
                res = iter;
                currWeight = get<WEIGHT>(res->second);
            }

            ++iter;
        }

        // Add edge to used list and delete form curr list
        EdgeTuple edge = res->second;
        m_edgeused.insert(*res);
        m_edgelist.erase(res);

        return edge;
    }


    // Private vars
private:
    // Parsed Information
    int m_nodesN;
    vector<string> m_nodes;
    EdgeMap m_edges;
    set<string> m_vistedNodes;

    // Container to build Forwarding table
    EdgeMap m_routingmap;
    EdgeMap m_edgelist;
    EdgeMap m_edgeused;
    EdgeMap m_forwardingTable;
    //set<string> m_notvisted;


};



int main()
{
    string filename;
    // Link State Routing
    cout << "Provide the test file you wish to use (ie filename.txt): " << endl;
    cin >> filename;


    LinkState lstate(filename);
    lstate.CreateRoutingTable();
    lstate.Print();

    cout << "Finish" << endl;
    return 0;
}