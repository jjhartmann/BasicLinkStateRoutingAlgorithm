#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <tuple>
#include <functional>

using namespace std;

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
                m_edges.push_back(tuple);

                pindex = index + 1;
            }
        }

    }

    // Private vars
private:
    int m_nodesN;
    vector<string> m_nodes;
    vector<tuple<string, string, int>> m_edges;

};



int main()
{
    string filename;
    // Link State Routing
    cout << "Provide the test file you wish to use (ie filename.txt): " << endl;
    cin >> filename;


    LinkState lstate(filename);



    cout << "Finish" << endl;
    return 0;
}