#include "ModelOne.h"

void Translator::read() {
	this->e_dictionary.insert(std::make_pair("", 0));
	this->f_dictionary.insert(std::make_pair("", 0));
	this->e_dictionary_inversed.insert(std::make_pair(0, ""));
	this->f_dictionary_inversed.insert(std::make_pair(0, ""));
	this->aligments.push_back({ 0 });
	this->body.push_back(std::make_pair(std::vector<int>(0), std::vector<int>(0)));
	this->l.push_back(1);
	this->m.push_back(1);
	while (!training_set.eof()) {

		//auxiliary containers		
		std::vector<int> sentense_one; //"English" sentense
		std::vector<int> sentense_two; //"French" sentense
		std::vector<int> aligments_one_two; //Aligments between "English" and "Franch" sentenses
		char sentense_buffer[512];
		std::string word;
		std::string sentense;
		int number;

		//read "English" sentense
		training_set.getline(sentense_buffer, 512);
		sentense = std::string(sentense_buffer);
		//std::cout << sentense << std::endl;
		sentense_one.push_back(0);
		for (int i = 0; i < sentense.size(); i++) {
			//std::cout << sentense[i];
			if (!isspace(sentense[i])) {
				word.push_back(sentense[i]);
			}
			else if(word.size()) {
				auto itr = this->e_dictionary.find(word);
				if (itr != this->e_dictionary.end()) {
					sentense_one.push_back(itr->second);
				}
				else {
					this->e_size++;
					this->e_dictionary.insert(std::make_pair(word, e_size));
					this->e_dictionary_inversed.insert(std::make_pair(e_size, word));
					sentense_one.push_back(e_size);
				}

				word.clear();
			}
		}
		if (word.size()) {
			auto itr = this->e_dictionary.find(word);
			if (itr != this->e_dictionary.end()) {
				sentense_one.push_back(itr->second);
			}
			else {
				this->e_size++;
				this->e_dictionary.insert(std::make_pair(word, e_size));
				this->e_dictionary_inversed.insert(std::make_pair(e_size, word));
				sentense_one.push_back(e_size);
			}
		}
		word.clear();
		//std::cout << std::endl;

		//read "French" sentense
		sentense_two.push_back(0);
		training_set.getline(sentense_buffer, 512);
		sentense = std::string(sentense_buffer);
		//std::cout << sentense << std::endl;
		for (int i = 0; i < sentense.size(); i++) {
			//std::cout << sentense[i];
			if (!isspace(sentense[i])) {
				word.push_back(sentense[i]);
			}
			else if (word.size()) {
				auto itr = this->f_dictionary.find(word);
				if (itr != this->f_dictionary.end()) {
					sentense_two.push_back(itr->second);
				}
				else {
					this->f_size++;
					this->f_dictionary.insert(std::make_pair(word, f_size));
					this->f_dictionary_inversed.insert(std::make_pair(f_size, word));
					sentense_two.push_back(f_size);
				}

				word.clear();
			}
		}
		if (word.size()) {
			auto itr = this->f_dictionary.find(word);
			if (itr != this->f_dictionary.end()) {
				sentense_two.push_back(itr->second);
			}
			else {
				this->f_size++;
				this->f_dictionary.insert(std::make_pair(word, f_size));
				this->f_dictionary_inversed.insert(std::make_pair(f_size, word));
				sentense_two.push_back(f_size);
			}
			word.clear();
		}
		this->body.push_back(std::make_pair(sentense_one, sentense_two));
		this->l.push_back(sentense_one.size());
		this->m.push_back(sentense_two.size());
		this->n++;
		//std::cout << std::endl;

		//read Aligments
		aligments_one_two.push_back(0);
		training_set.getline(sentense_buffer, 512);
		sentense = std::string(sentense_buffer);
		//std::cout << sentense << std::endl;
		//for (int i = 0; i < sentense.size(); i++) {
			//std::cout << sentense[i];
		//}
		//std::cout << std::endl;
		/*for (int i = 0; i < sentense.size(); i++) {
			if (!isspace(sentense[i])) {
				word.push_back(sentense[i]);
			}
			else if (word.size()) {
				aligments_one_two.push_back(atoi(word.c_str()));
				word.clear();
			}
		} if (word.size()) {
			aligments_one_two.push_back(atoi(word.c_str()));
			word.clear();
		}
		this->aligments.push_back(aligments_one_two);*/
	}	
}

void Translator::initialize_delta() {
	this->delta.resize(n+1);
	for (int k = 0; k <= this->n; k++) {
		delta[k].resize(this->m[k]);
		for (int i = 1; i < this->m[k]; i++) {
			delta[k][i].resize(this->l[k]);
			/*for (int j = 0; j < this->l[k]; j++) {
				if (this->aligments[k][i] == j)
					delta[k][i][j] = 1;
				else
					delta[k][i][j] = 0;
				
			}*/
		}
	}
}


void Translator::initialize_c() {
	e_mutex.reserve(this->e_size + 1);
	for (int i = 0; i <= this->e_size; i++) {
		this->e_mutex.push_back(std::shared_ptr<std::mutex>(new std::mutex));
	}
	/*e_locks.reserve(this->e_size + 1);
	for (int i = 0; i <= this->e_size; i++) {
		this->e_locks.push_back(std::shared_ptr<spin_lock>());
	}*/
	this->c_e.resize(this->e_size + 1, 0);
	this->c_e_f.resize(this->e_size + 1);
	for (int i = 0; i <= this->e_size; i++) {
		this->c_e_f[i].resize(this->f_size + 1,0);
	}
}

void Translator::compute_c() {
	std::vector<std::thread> threads;
	for (int h = 0; h < this->max_threads; h++) {
		threads.push_back(std::thread([this, &h]{
			for (int k = h+1; k <= this->n; k+=this->max_threads) {
				for (int i = 1; i < this->m[k]; i++) {
					for (int j = 0; j < this->l[k]; j++) {
						this->e_mutex[this->body[k].first[j]]->lock();
						//this->e_locks[this->body[k].first[j]]->lock();
						this->c_e[this->body[k].first[j]] += delta[k][i][j];
						this->c_e_f[this->body[k].first[j]][this->body[k].second[i]] += delta[k][i][j];
						//this->e_locks[this->body[k].first[j]]->unlock();
						this->e_mutex[this->body[k].first[j]]->unlock();
					}
				}
			}
		}));
	}
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}
}

void Translator::compute_t() {
	std::vector<std::thread> threads;
	for (int h = 0; h < this->max_threads; h++) {
		threads.push_back(std::thread([this, &h]{
			for (int i = h+1; i <= this->f_size; i+=this->max_threads) {
				for (int j = 0; j <= this->e_size; j++) {
					if (c_e[j] == 0)
						t[i][j] = 0;
					else
						t[i][j] = c_e_f[j][i] / c_e[j];
				}
			}
		}));		
	}
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}
}

void Translator::translate(std::string input) {
	std::vector<int> encryption;
	std::string word;
	for (int i = 0; i < input.size(); i++) {
		if (!isspace(input[i])) {
			word.push_back(input[i]);
		}
		else if (word.size()) {
			auto itr = this->f_dictionary.find(word);
			if (itr != this->f_dictionary.end()) {
				encryption.push_back(itr->second);
			}
			else {
				std::cout << "ERROR" << std::endl;
			}
			word.clear();
		}
	}
	if (word.size()) {
		auto itr = this->f_dictionary.find(word);
		if (itr != this->f_dictionary.end()) {
			encryption.push_back(itr->second);
		}
		else {
			std::cout << "ERROR" << std::endl;
		}
	}
	word.clear();

	std::vector<int> translation;
	for (int i = 0; i < encryption.size(); i++) {
		double max = 0;
		int number = 0;
		for (int j = 0; j < this->t[encryption[i]].size(); j++) {
			if (t[encryption[i]][j] > max) {
				max = t[encryption[i]][j];
				number = j;
			}
		}
		translation.push_back(number);
	}

	for (int i = 0; i < translation.size(); i++) {
		std::cout << e_dictionary_inversed.find(translation[i])->second << " ";
	}
}

void Translator::initialize_t() {
	t.resize(f_size + 1);
	for (int i = 0; i <= this->f_size; i++) {
		t[i].resize(e_size + 1, 0.3);
	}
}

float Translator::run() {
	this->read();
	float time_start = clock() / (float) CLOCKS_PER_SEC;
	std::thread init_c([this]{
		this->initialize_c();
	});
	std::thread init_delta([this]{
		this->initialize_delta();
	});
	std::thread init_t([this]{
		this->initialize_t();
	});
	init_c.join();
	init_delta.join();
	init_t.join();
	for (int s = 0; s < 5; s++) {
		this->reset_c();
		this->compute_delta();
		this->compute_c();
		this->compute_t();
	}
	this->align_t();
	float time_finish = clock() / (float) CLOCKS_PER_SEC;
	return time_finish - time_start;
}

void Translator::reset_c() {
	std::vector<std::thread> threads;
	threads.push_back(std::thread([this]{
		for (int i = 0; i < this->c_e.size(); i++) {
			this->c_e[i] = 0;
		}
	}));
	for (int h = 0; h < this->max_threads - 1; h++) {
		threads.push_back(std::thread([this, h] {
			for (int i = h; i < this->c_e_f.size(); i += this->max_threads - 1) {
				for (int j = 0; j < this->c_e_f[i].size(); j++) {
					this->c_e_f[i][j] = 0;
				}
			}
		}));
	}
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}
}

void Translator::compute_delta() {
	std::vector<std::thread> threads;
	for (int h = 0; h < this->max_threads; h++) {
		threads.push_back(std::thread([this, &h]{
			for (int k = h+1; k <= this->n; k+=this->max_threads) {
				for (int i = 1; i < this->m[k]; i++) {
					double sum_of_t = 0;
					for (int j = 0; j < this->l[k]; j++) {
						sum_of_t += this->t[this->body[k].second[i]][this->body[k].first[j]];
					}
					for (int j = 0; j < this->l[k]; j++) {
						this->delta[k][i][j] = this->t[this->body[k].second[i]][this->body[k].first[j]] /
							sum_of_t;
					}
				}
			}
		}));
	}
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}
}

void Translator::align_t() {
	std::vector<std::thread> threads;
	for (int h = 0; h < this->max_threads; h++) {
		threads.push_back(std::thread([this, &h]{
			for (int i = h + 1; i <= this->f_size; i += this->max_threads) {
				double sum_of_t = 0;
				for (int j = 0; j <= this->e_size; j++) {
					sum_of_t += t[i][j];
				}
				for (int j = 0; j <= this->e_size; j++) {
					t[i][j] /= sum_of_t;
				}
			}
		}));
	}
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}
}

void Translator::print_result() {
	std::vector<std::vector<std::pair<float, int>>> t_vec;
	for (int i = 0; i <= this->f_size; i++) {
		std::vector<std::pair<float, int>> t_vec_buf;
		for (int j = 0; j <= this->e_size; j++)
			t_vec_buf.push_back(std::make_pair(t[i][j], j));
		t_vec.push_back(t_vec_buf);
	}
	for (int i = 0; i < t_vec.size(); i++) {
		std::sort(t_vec[i].begin(), t_vec[i].end(), [](std::pair<float, int> a, std::pair<float, int> b){
			return b.first < a.first;
		});
	}
	for (int i = 1; i < t_vec.size(); i++) {
		this->output << this->f_dictionary_inversed.find(i)->second << " : " << std::endl;
		int j = 0;
		while (t_vec[i][j].first > 0.01 && j < t_vec[i].size()) {
			output <<"\t" << this->e_dictionary_inversed.find(t_vec[i][j].second)->second << " "
				<< t_vec[i][j].first << " %" << std::endl;
			j++;
		}
	}
}