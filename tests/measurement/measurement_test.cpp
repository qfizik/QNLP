
#include "util/tinymatrix.hpp"
#include "QubitCircuit.hpp"



int main(int argc, char **argv){

    openqu::mpi::Environment env(argc, argv);
    if (env.is_usefull_rank() == false) return 0;

    int rank = env.rank();

    int num_exps, m;
    std::size_t n;
    n = 2;                     // Number of qubit registers
    num_exps = 1000;
    m = pow(2,n);       // Number of possible states

    vector<int> count(m);

    QubitCircuit<ComplexDP> circ(n);

    // Repeated shots of experiment
    for(int exp = 0; exp < num_exps; exp++){

        circ.Initialize("base",0);
         
        circ.ApplyHadamard(0);
        circ.ApplyHadamard(1);

        for(std::size_t j = 0; j < n; j++){
            if(!circ.IsClassicalBit(j)){
                cout << j << ": I am entangled" << endl;
            }
            circ.ApplyMeasurement(j);
        }

        // Obtain the probability of state being |1>
        vector<double> output(n);
        // Obtain the measured qubit 
        for(std::size_t j = 0; j < n; j++){
            //output[j] = circ.GetClassicalValue(j);
            output[j] = circ.GetProbability(j);
        }

        // Increase the count of the pattern being measured
        int tmp_count;
        if(output[0] == 0){
            if(output[1] == 0){
                count[0]++;
            }
            else if(output[1] == 1){
                count[1]++;
            }
        }
        if(output[0] == 1){
            if(output[1] == 0){
                count[2]++;
            }
            else if(output[1] == 1){
                count[3]++;
            }
        }

     //   cout << exp << ":\t" << output[0] << "\t" << output[1] << endl;
    
    }
            
    cout << "Measure:" << endl;
    for(int i = 0; i < m; i++){
        cout << i << '\t' << count[i] << '\t' << 100.0*(double)count[i]/(double) num_exps << "%" <<  endl;
    }


    std::vector<unsigned>qubits_to_be_measured={0,1};
    // Paulimatrices(X=1,Y=2,Z=3)
    std::vector<unsigned>observables={3,3};

    double average=0.0;
    circ.ExpectationValue(qubits_to_be_measured,observables,average);
    cout << "Expectation\t" << average << endl;


    // Generate |00>, |01>, |10>, |11>
    QubitRegister<ComplexDP> psi1(2, "base", 0);
    QubitRegister<ComplexDP> psi2(2, "base", 0);
    QubitRegister<ComplexDP> psi3(2, "base", 0);
    QubitRegister<ComplexDP> psi4(2, "base", 0);

    psi2.ApplyPauliX(0);
    psi3.ApplyPauliX(1);
    psi4.ApplyPauliX(0);
    psi4.ApplyPauliX(1);

    ComplexDP overlap1, overlap2,overlap3,overlap4;

    overlap1 = circ.ComputeOverlap(psi1);
    overlap2 = circ.ComputeOverlap(psi2);
    overlap3 = circ.ComputeOverlap(psi3);
    overlap4 = circ.ComputeOverlap(psi4);

    cout << "Overlaps" << endl;
    cout << "<00|psi> = " << overlap1 << "\t";
    cout << "<10|psi> = " << overlap2 << "\t";
    cout << "<01|psi> = " << overlap3 << "\t";
    cout << "<11|psi> = " << overlap4 << endl;




    cout << "qubit\t\t0\t1" << endl;
    for(std::size_t j = 0; j < n; j++){

        cout << "register" << j << "\t" <<  1.0 - circ.GetProbability(j) << "\t" <<  circ.GetProbability(j)  << endl;
    }

    return 0;
}
    
