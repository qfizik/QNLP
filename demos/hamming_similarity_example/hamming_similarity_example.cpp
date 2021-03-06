/**
 * @file hamming_similarity_example.cpp
 * @author Myles Doyle (myles.doyle@ichec.ie)
 * @brief Compute the similarity of a test state to each state in a small corpus. This demo shows a simple mapping of a corpus to a basis set, then computes the similarity of a test state to the corpus in terms of the basis set. A superposition of training states are initially encoded, the Hamming distance is computed and Y-rotations executed on an auxiliary qubit to proportionally adjust the amplitudes corresponding to the similarity of each training state to the test state. A single state is then measured. This is repeated to build a distribution. 
 * @version 0.1
 * @date 2020-01-28
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "IntelSimulator.cpp"  
#include "Simulator.hpp"  
#include <map>
#include <iterator>
#include <unordered_map>

using namespace QNLP;

struct Basis{
    std::unordered_map<std::string, std::size_t> token_to_bin_ns, token_to_bin_v, token_to_bin_no;

    Basis() :   token_to_bin_ns{ {"adult",0}, {"child",1}, {"smith",2}, {"surgeon",3} },
                token_to_bin_v { { "stand", 0 }, { "move", 1 }, { "sit", 2 }, { "sleep", 3 } }, 
                token_to_bin_no { { "inside", 0 }, { "outside", 1 } }
        {};
    ~Basis() {};
};

struct Statement{
    Basis basis;
    std::string text;
    std::vector<std::size_t> state_patterns;
    Statement() {};
    ~Statement() {};;
};

struct Data{
    Statement s;
    std::vector<std::size_t> state_patterns;
    Data(){};
    ~Data() {};
};

/**
 * @brief Checks if the bit'th bit of the integer byte is set
 * 
 * @param byte - integer representing a binary string
 * @param bit - the index of the bit to be checked (beginning
 * with the least significant bit)
 */
#define IS_SET(byte,bit) (((byte) & (1UL << (bit))) >> (bit))

/**
 * @brief Print binary representation of input value of length len
 * 
 * @param val - positive integer input
 * @param len - length of the binary string to be printed beginning at LSB.
 */
void print_bits(unsigned int val, int len){
    for (int i = len-1; i > -1; i--){
        std::cout << ((val >> i) & 1UL);
    }
}

int main(int argc, char **argv){

    bool verbose = false;

    if(argc > 1){
        verbose = atoi(argv[1]);
    }
    std::size_t num_exps = 100;

    std::size_t len_reg_memory = 5;
    std::size_t len_reg_auxiliary = len_reg_memory + 2;
    std::size_t num_qubits = len_reg_memory + len_reg_auxiliary;;
    std::size_t num_bin_pattern = 8;

    std::size_t test_pattern = 0;

    if(argc > 2){
        test_pattern = atoi(argv[2]);
    }

    if(argc > 3){
        num_exps = atoi(argv[3]);
    }

    SimulatorGeneral<IntelSimulator> *sim = new IntelSimulator(num_qubits);

    int rank;
#if ENABLE_MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#else
    rank = 0;
#endif


    // Set up registers to store indices
    std::vector<std::size_t> reg_memory(len_reg_memory);
    for(std::size_t i = 0; i < len_reg_memory; i++){
        reg_memory[i] = i;
    }
    std::vector<std::size_t> reg_auxiliary(len_reg_auxiliary);
    for(std::size_t i = 0; i < len_reg_auxiliary; i++){
        reg_auxiliary[i] = i + len_reg_memory;
    }

    // Init data to encode
    //std::vector<std::size_t> vec_to_encode {5, 7, 8, 10, 21, 23, 24, 26};
    std::vector<std::size_t> vec_to_encode {4,6,20,22,9,11,25,27};

    std::size_t val = 0;

    // Init result counter
    std::map<std::size_t, std::size_t> count;
    for(std::size_t i = 0; i < vec_to_encode.size(); i++){
        count.insert(pair<std::size_t, std::size_t>(vec_to_encode[i],0));
    }

    // Repeated shots of experiment
    for(int exp = 0; exp < num_exps; exp++){

        sim->initRegister();

        // Encode
        #ifdef GATE_LOGGING
        sim->getGateWriter().segmentMarkerOut("Encode");
        #endif
        sim->encodeBinToSuperpos_unique(reg_memory, reg_auxiliary, vec_to_encode, len_reg_memory); 

        if(verbose){
            sim->PrintStates("After encoding: ");
        }

        
        //std::cout << "PRE-HAM PROB[" << reg_auxiliary[len_reg_auxiliary-2] << "]=1    :" << sim->getQubitRegister().GetProbability(reg_auxiliary[len_reg_auxiliary-2]) << std::endl;


        // Compute Hamming distance between test pattern and encoded patterns
        #ifdef GATE_LOGGING
        sim->getGateWriter().segmentMarkerOut("Compute Hamming distance");
        #endif
        sim->applyHammingDistanceRotY(test_pattern, reg_memory, reg_auxiliary, len_reg_memory);

        if(verbose){
            sim->PrintStates("After Hamming Rot_Y: ");
        }

        //std::cout << "POST-HAM PROB[" << reg_auxiliary[len_reg_auxiliary-2] << "]=1    :" << sim->getQubitRegister().GetProbability(reg_auxiliary[len_reg_auxiliary-2]) << std::endl;


        // Measure
        sim->collapseToBasisZ(reg_auxiliary[len_reg_auxiliary-2], 1);
        if(verbose){
            sim->PrintStates("After Collapse to Basis z: ");
        }

        //std::cout << "POST-Z PROB[" << reg_auxiliary[len_reg_auxiliary-2] << "]=1    :" << sim->getQubitRegister().GetProbability(reg_auxiliary[len_reg_auxiliary-2]) << std::endl;

        val = sim->applyMeasurementToRegister(reg_memory);
        if(verbose){
            sim->PrintStates("After Measurement: ");
        }
        //sim->applyGateX(reg_auxiliary[len_reg_auxiliary-2]);

        count[val] += 1;
    }

    cout << "Measure:" << endl;
    int i = 0;
    //Create reverse dict
    std::map<std::size_t, std::size_t> count_vk;
    for(auto& [k,v] : count){
        count_vk[v] = k;
    }
    for(auto& [k,v] : count_vk){
        //cout << vec_to_encode[i] << "\t";
        cout << v << "\t";
        cout << "|";
        print_bits(v, len_reg_memory);
        cout << ">\t";
        cout << k << "\t" << ((double) k / (double) num_exps) << endl;
        i++;
    }

    return 0;
}
    
