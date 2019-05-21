#include "qft.hpp"
#include "sum_sub.hpp"
#include "qureg/qureg.hpp"
#include <cmath>

using namespace QNLP;

void SumSub::sum_reg(ISimulator& qReg, const unsigned int r1_min, const unsigned int r1_max, const unsigned int r2_min, const unsigned int r2_max){
    std::size_t num_qubits_r1 = r1_max - r1_min;
    std::size_t num_qubits_r2 = r2_max - r2_min;

    assert( num_qubits_r1 == num_qubits_r2 );

    QFT::applyQFT(qReg, r2_min, r2_max);
    for(int i = num_qubits_r2; i > 0; i--){
        for(int j = i; j > 0; j--){
            qReg.applyGateCPhaseShift(2.0*M_PI / (1<<(1 + (i-j))), r1_min + (j-1), r2_min + (i-1));
        }
    }
    QFT::applyIQFT(qReg, r2_min, r2_max);
}

void SumSub::sub_reg(ISimulator& qReg, const unsigned int r1_min, const unsigned int r1_max, const unsigned int r2_min, const unsigned int r2_max){
    std::size_t num_qubits_r1 = r1_max - r1_min;
    std::size_t num_qubits_r2 = r2_max - r2_min;

    assert( num_qubits_r1 == num_qubits_r2 );

    //Flip all states in r1
    for(int i = r1_min; i < r1_max; i++){
        qReg.applyGateX(i);
    }
    SumSub::sum_reg(qReg, r1_min, r1_max, r2_min, r2_max);
    //Flip states to return the sum
    for(int i = r2_min; i < r2_max; i++){
        qReg.applyGateX(i);
    }    
    //Return all qubits in r1 to original state 
    for(int i = r1_min; i < r1_max; i++){
        qReg.applyGateX(i);
    }
}
