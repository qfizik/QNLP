//#define CATCH_CONFIG_RUNNER

#include "bin_into_superpos.hpp"
#include "Simulator.hpp"
#include "IntelSimulator.cpp"
#include "catch2/catch.hpp"

#include <bitset>

using namespace QNLP;

//For simplicity, enabling complex double only
typedef ComplexDP Type;

template class EncodeBinIntoSuperpos<IntelSimulator>;



TEST_CASE("Test encoding of binary (integers) to superposition","[encode]"){
    std::size_t num_qubits = 3, num_bin_pattern = 5, num_exps = 100;
    std::size_t len_bin_patterns = num_qubits;

    IntelSimulator sim(num_qubits);
    REQUIRE(sim.getNumQubits() == num_qubits);

    // Set up registers
    std::vector<std::size_t>> reg_memory(num_qubits);
    for(std::size_t i = 0; i < len_bin_pattern; i++){
        reg_memory[i] = i;
    }
    std::vector<std::size_t>> reg_ancilla(len_bin_pattern + 2);
    for(std::size_t i = 0; i < len_bin_pattern + 2; i++){
        reg_ancilla[i] = i + len_bin_pattern;
    }

    // Init data to encode
    std::vector<std::size_t>> vec_to_encode(num_bin_pattern);
    for(std::size_t i = 0; i < num_bin_pattern; i++){
        vec_to_encode[i] = i;
    }

    std::size_t val;
    for(size_t exp = 0; exp < num_exps; exp++){
        // Encode
        EncodeBinIntoSuperpos<decltype(sim)> encoder(num_bin_pattern, len_bin_patterns);
        encoder.encodeBinInToSuperpos(sim, reg_memory, reg_ancilla, vec_to_encode);

        // Measure
        val = sim.applyMeasurement(reg_memory, len_bin_patterns);

        CHECK_THAT(val, VectorContains(vec_to_encode))

    }
}

/*
TEST_CASE("Test encoding of binary (integers) to superposition","[encode]"){
    for(std::size_t num_qubits=3; num_qubits <=8; num_qubits++){
        std::size_t c_start = 0, c_end = num_qubits-2, target = num_qubits-1;
        DYNAMIC_SECTION("Testing " << num_qubits << " qubits"){
            IntelSimulator sim(num_qubits);
            REQUIRE(sim.getNumQubits() == num_qubits);
            std::size_t pattern_total = 1<<(num_qubits-1);
            for(std::size_t pattern_idx = 0; pattern_idx < pattern_total; pattern_idx++ ){
                sim.initRegister();
                DYNAMIC_SECTION("Testing pattern " << pattern_idx){
                    for (int ctrl_qubit = 0; ctrl_qubit < num_qubits-1; ctrl_qubit++){
                        unsigned int window_select = (pattern_idx >> ctrl_qubit) & 0b1;
                        if(window_select == 1){
                            sim.applyGateX(ctrl_qubit);
                        }
                    }
                    NCU<decltype(sim)> nqd(sim.getGateX(), num_qubits-1);
                    nqd.applyNQubitControl(sim, c_start, c_end, target, sim.getGateX(), 0, true); //isX = true

                    if(pattern_idx != pattern_total-1){
                        REQUIRE(sim.getQubitRegister().GetProbability( target )  == Approx(0.0).margin(1e-12));
                    }
                    else{
                        REQUIRE(sim.getQubitRegister().GetProbability( target )  == Approx(1.0).margin(1e-12));
                    }
                }
            }

        }

    }
}

TEST_CASE("Test n-controlled unitary simulator methods","[ncu]"){
    for(std::size_t num_qubits=3; num_qubits <=8; num_qubits++){
        std::size_t c_start = 0, c_end = num_qubits-2, target = num_qubits-1;
        DYNAMIC_SECTION("Testing " << num_qubits << " qubits"){
            IntelSimulator sim(num_qubits);
            REQUIRE(sim.getNumQubits() == num_qubits);
            std::size_t pattern_total = 1<<(num_qubits-1);
            for(std::size_t pattern_idx = 0; pattern_idx < pattern_total; pattern_idx++ ){
                sim.initRegister();
                DYNAMIC_SECTION("Testing pattern " << pattern_idx){
                    for (int ctrl_qubit = 0; ctrl_qubit < num_qubits-1; ctrl_qubit++){
                        unsigned int window_select = (pattern_idx >> ctrl_qubit) & 0b1;
                        if(window_select == 1){
                            sim.applyGateX(ctrl_qubit);
                        }
                    }
                    sim.applyGateNCU<decltype(sim.getGateX())>(sim.getGateX(), c_start, c_end, target);
                    if(pattern_idx != pattern_total-1){
                        REQUIRE(sim.getQubitRegister().GetProbability( target )  == Approx(0.0).margin(1e-12));
                    }
                    else{
                        REQUIRE(sim.getQubitRegister().GetProbability( target )  == Approx(1.0).margin(1e-12));
                    }
                }
            }

        }

    }
}
*/
/*
int main( int argc, char* argv[] ) {
    openqu::mpi::Environment env(argc, argv);

    int result = Catch::Session().run( argc, argv );

    return result;
}
*/
