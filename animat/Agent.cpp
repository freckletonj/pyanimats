// Agent.cpp

#include <math.h>
#include <vector>

#include "./Agent.hpp"

#define randDouble ((double)rand() / (double)RAND_MAX)


Agent::Agent(vector<unsigned char> genome) : genome(genome) {
    // state = states of sensors, hiddens, motors
    for (int i = 0; i < NUM_NODES; i++) {
        states[i] = 0;
        newStates[i] = 0;
    }
    gen = 0;
    correct = 0;
    incorrect = 0;
    hmms.clear();
}

Agent::~Agent() {
    for (int i = 0; i < (int)hmms.size(); i++) {
        delete hmms[i];
    }
}

void Agent::resetState() {
    for (int i = 0; i < NUM_NODES; i++)
        states[i] = 0;
}

void Agent::updateStates() {
    // 
    for (int i = 0; i < (int)hmms.size(); i++) {
        hmms[i]->update(&states[0], &newStates[0]);
    }
    for (int i = 0; i < NUM_NODES; i++) {
        states[i] = newStates[i];
        newStates[i] = 0;
    }
}

void Agent::generatePhenotype() {
    if (hmms.size() != 0) {
        for (int i = 0; i < (int)hmms.size(); i++) {
            delete hmms[i];
        }
    }
    hmms.clear();
    HMM *hmm;
    for (int i = 0; i < (int)genome.size(); i++) {
        if ((genome[i] == 42) && (genome[(i + 1) % (int)genome.size()] == 213)) {
            hmm = new HMM(genome, i);
            hmms.push_back(hmm);
        }
    }
}

void Agent::mutateGenome(double mutProb, double dupProb, double delProb,
        int minGenomeLength, int maxGenomeLength) {
    // Mutation
    for (int i = 0; i < (int)genome.size(); i++) {
        if (randDouble < mutProb) {
            genome[i] = rand() & 255;
        }
    }
    // Duplication
    if ((randDouble < dupProb) && ((int)genome.size() < maxGenomeLength)) {
        int width = (MIN_DUP_DEL_LENGTH + rand()) & MAX_DUP_DEL_LENGTH;
        int start = rand() % ((int)genome.size() - width);
        int insert = rand() % (int)genome.size();
        vector<unsigned char> buffer;
        buffer.clear();
        buffer.insert(buffer.begin(), genome.begin() + start, genome.begin() +
                start + width);
        genome.insert(genome.begin() + insert, buffer.begin(), buffer.end());
    }
    // Deletion
    if ((randDouble < delProb) && ((int)genome.size() > minGenomeLength)) {
        int width = (MIN_DUP_DEL_LENGTH + rand()) & MAX_DUP_DEL_LENGTH;
        int start = rand() % ((int)genome.size() - width);
        genome.erase(genome.begin() + start, genome.begin() + start + width);
    }
}

void Agent::injectStartCodons(int n) {
    for (int i = 0; i < (int)genome.size(); i++)
        genome[i] = rand() & 255;
    for (int i = 0; i < n; i++) {
        int j = rand() % ((int)genome.size() - 100);
        // Start codon is a 42 followed by a 213.
        genome[j] = 42;
        genome[j + 1]= 213;
        for (int k = 2; k < 20; k++)
            genome[j + k] = rand() & 255;
    }
}

vector< vector<int> > Agent::getEdges() {
    vector< vector<int> > edgeList;
    edgeList.clear();
    vector<int> edge;
    for (int i = 0; i < (int)hmms.size(); i++) {
        for (int j = 0; j < (int)hmms[i]->ins.size(); j++) {
            for (int k = 0; k < (int)hmms[i]->outs.size(); k++) {
                edge.clear();
                edge.resize(2);
                edge[0] = hmms[i]->ins[j];
                edge[1] = hmms[i]->outs[k];
                edgeList.push_back(edge);
            }
        }
    }
    return edgeList;
}


vector< vector<bool> > Agent::getTransitions() {
    // Save animats original state.
    unsigned char initial_states[NUM_NODES];
    for (int i = 0; i < NUM_NODES; i++) {
        initial_states[i] = states[i];
    }
    vector< vector<bool> > tpm;
    tpm.clear();
    tpm.resize(NUM_STATES);
    for (int i = 0; i < NUM_STATES; i++) {
        // Set animat to the ith state (using LOLI mapping from states to
        // integers).
        for (int j = 0; j < NUM_NODES; j++) {
            states[j] = (i >> j) & 1;
        }
        // Update the state to get the transition and record it.
        updateStates();
        tpm[i].resize(NUM_NODES);
        for (int j = 0; j < NUM_NODES; j++) {
            tpm[i][j] = states[j];
        }
    }
    // Return animat to its original state.
    for (int i = 0; i < NUM_NODES; i++) {
        states[i] = initial_states[i];
    }
    return tpm;
}
