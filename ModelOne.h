#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <memory>
#include <algorithm>
#include <functional>
#include <atomic>

class Translator {
public:
	/*class spin_lock{
		std::atomic<unsigned int> m_spin;
	public:
		spin_lock() : m_spin(0){}
		~spin_lock() {
			assert(m_spin.load(std::memory_order_relaxed) == 0);
		}
		void lock() {
			unsigned int nCur;
			do {
				nCur = 0;
			} while (!m_spin.compare_exchange_weak(nCur, 1, std::memory_order_acquire));
		}
		void unlock() {
			m_spin.store(0, std::memory_order_release);
		}
	};*/

	Translator(std::string input_file, std::string output_file, int cores) : 
		training_set(input_file), max_threads(cores), output(output_file){}
	void read();
	float run();
	void initialize_delta();
	void initialize_c();
	void initialize_t();
	void compute_c();
	void compute_t();
	void reset_c();
	void compute_delta();
	void translate(std::string input);
	void align_t();
	void print_result();
private:
	//Dictionaries
	std::map<std::string, int> e_dictionary;
	std::map<std::string, int> f_dictionary;
	std::map<int, std::string> e_dictionary_inversed;
	std::map<int, std::string> f_dictionary_inversed;

	std::vector<std::vector<int>> aligments;
	std::vector < std::pair<std::vector<int>, std::vector<int>>> body;

	std::vector<int> l; //number of words in the "English" sentence 
	std::vector<int> m; //number of words in the "French" sentence
	int n; //number of sentenses

	int e_size;// size of "English" dictionary;
	int f_size;// size of "French" dictionary;

	//FILES
	std::ifstream training_set;
	std::ofstream output;

	//EM-parametrs
	std::vector<std::vector<std::vector<double>>> delta;
	std::vector<std::vector<double>> t;// t(f|e) So, it's what we need
	std::vector<double> c_e;// c(e)
	std::vector<std::vector<double>> c_e_f;// c(e,f)
	
	//Parallel utils
	int max_threads;
	std::vector<std::shared_ptr<std::mutex>> e_mutex; // one Mutex for each "English" word
	//std::vector<std::shared_ptr<spin_lock>> e_locks; //one Lock for each "English" word
	//Advanced EM-parametrs for Model two, but in Model one it's optional.
	//std::vector<std::vector<std::vector<std::vector<double>>>>  q;
	//std::vector<std::vector<std::vector<std::vector<double>>>> c_j_ilm;
	//std::vector<std::vector<std::vector<double>>> c_ilm;

};