#include "sha256.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <list>
#include <tuple>
using namespace std;

struct pair_hash {
	template <class T1, class T2>
	std::size_t operator () (const std::pair<T1,T2> &p) const {
		auto h1 = std::hash<T1>{}(p.first);
		auto h2 = std::hash<T2>{}(p.second);
		return h1 ^ h2;  
	}
};

SHA256 sha256;

const int start_of_something_new = 1231006505;
string coinbase = "0000000000000000000000000000000000000000000000000000000000000000";
uint64_t reward[9] = {5000000000, 2500000000, 1250000000, 625000000, 312500000, 156250000, 78125000, 39062500, 19531250};

unsigned char header[100];
unsigned char block[100000000];
int index;

string prev_hash; // 지금 할 차례인 block의 prev_hash
int block_height; // 지금 할 차례인 block의 높이
int week; // 이번 주는 week주차
vector<pair<uint64_t, int>> burned;
vector<tuple<uint64_t, uint32_t, int>> john;

int64_t utxo_sum;
int64_t utxo_3year_sum;
int64_t utxo_5year_sum;
int64_t utxo_10year_sum;

int utxo_cnt;
int total_tx_count;
int prev_total_tx_count;
int start_block_height;

// utxo: (txhash, txindex): (value, timestamp)
unordered_map<pair<string, uint32_t>, uint64_t, pair_hash> utxo;
unordered_map<pair<string, uint32_t>, uint64_t, pair_hash> utxo_3year;
unordered_map<pair<string, uint32_t>, uint64_t, pair_hash> utxo_5year;
unordered_map<pair<string, uint32_t>, pair<uint64_t, uint32_t>, pair_hash> utxo_10year;

list<tuple<uint32_t, string, uint32_t>> recent;
list<tuple<uint32_t, string, uint32_t>> recent_3year;
list<tuple<uint32_t, string, uint32_t>> recent_5year;

#define MAX_COLD_SIZE 100000

void flush() {
	ofstream out;
	out.open("info/utxo_cold", fstream::out | fstream::app);
	for (auto it = utxo_10year.begin(); it != utxo_10year.end(); ++it) {
		out << it->first.first << ' ' << it->first.second << ' ';
		out << it->second.first << ' ' << it->second.second << '\n';
	}
	utxo_10year.clear();
	out.close();
}

void restore() {
	ifstream in;
	in.open("info/basic");
	in >> prev_hash;
	in >> block_height;
	in >> week;

	int sz;
	cin >> sz;
	for (int i = 0; i < sz; ++i) {
		uint64_t a;
		int b;
		cin >> a >> b;
		burned.push_back({a, b});
	}

	cin >> sz;
	for (int i = 0; i < sz; ++i) {
		uint64_t a;
		uint32_t b;
		int c;
		cin >> a >> b >> c;
		john.push_back({a, b, c});
	}

	in >> utxo_sum;
	in >> utxo_3year_sum;
	in >> utxo_5year_sum;
	in >> utxo_10year_sum;

	in >> utxo_cnt;
	in >> total_tx_count;
	in >> prev_total_tx_count;
	in >> start_block_height;
	in.close();

	string s;
	uint32_t t, time;
	uint64_t value;

	in.open("info/utxo");
	while (in >> s >> t >> value) utxo[{s, t}] = value;
	in.close();

	in.open("info/utxo_3");
	while (in >> s >> t >> value) utxo_3year[{s, t}] = value;
	in.close();

	in.open("info/utxo_5");
	while (in >> s >> t >> value) utxo_5year[{s, t}] = value;
	in.close();

	in.open("info/list");
	while (in >> time >> s >> t) recent.push_back({time, s, t});
	in.close();

	in.open("info/list_3");
	while (in >> time >> s >> t) recent_3year.push_back({time, s, t});
	in.close();

	in.open("info/list_5");
	while (in >> time >> s >> t) recent_5year.push_back({time, s, t});
	in.close();
}

void save() {
	ofstream out;
	out.open("info/basic");
	out << prev_hash << '\n';
	out << block_height << '\n';
	out << week << '\n';

	out << burned.size() << '\n';
	for (int i = 0; i < burned.size(); ++i) {
		out << burned[i].first << ' ' << burned[i].second << '\n';
	}

	out << john.size() << '\n';
	for (int i = 0; i < john.size(); ++i) {
		out << get<0>(john[i]) << ' ' << get<1>(john[i]) << ' ' << get<2>(john[i]) << '\n';
	}

	out << utxo_sum << '\n';
	out << utxo_3year_sum << '\n';
	out << utxo_5year_sum << '\n';
	out << utxo_10year_sum << '\n';

	out << utxo_cnt << '\n';
	out << total_tx_count << '\n';
	out << prev_total_tx_count << '\n';
	out << start_block_height << '\n';
	out.close();

	out.open("info/utxo");
	for (auto it = utxo.begin(); it != utxo.end(); ++it) {
		out << it->first.first << ' ' << it->first.second << ' ';
		out << it->second << '\n';
	}
	out.close();

	out.open("info/utxo_3");
	for (auto it = utxo_3year.begin(); it != utxo_3year.end(); ++it) {
		out << it->first.first << ' ' << it->first.second << ' ';
		out << it->second << '\n';
	}
	out.close();

	out.open("info/utxo_5");
	for (auto it = utxo_5year.begin(); it != utxo_5year.end(); ++it) {
		out << it->first.first << ' ' << it->first.second << ' ';
		out << it->second << '\n';
	}
	out.close();

	flush();

	out.open("info/list");
	for (auto it = recent.begin(); it != recent.end(); ++it) {
		out << get<0>(*it) << ' ' << get<1>(*it) << ' ' << get<2>(*it) << '\n';
	}
	out.close();

	out.open("info/list_3");
	for (auto it = recent_3year.begin(); it != recent_3year.end(); ++it) {
		out << get<0>(*it) << ' ' << get<1>(*it) << ' ' << get<2>(*it) << '\n';
	}
	out.close();

	out.open("info/list_5");
	for (auto it = recent_5year.begin(); it != recent_5year.end(); ++it) {
		out << get<0>(*it) << ' ' << get<1>(*it) << ' ' << get<2>(*it) << '\n';
	}
	out.close();
}

pair<uint64_t, uint32_t> erase_in_cold(string txhash, uint32_t txindex, uint32_t timestamp) {
	pair<uint64_t, uint32_t> ret;
	if (utxo_10year.count({txhash, txindex})) {
		ret = utxo_10year[{txhash, txindex}];
		utxo_10year.erase({txhash, txindex});
	}
	else {
		string s;
		uint32_t t, time;
		uint64_t value;

		ifstream in;
		ofstream out;
		in.open("info/utxo_cold");
		out.open("info/temp");
		while (in >> s >> t >> value >> time) {
			if (s == txhash && t == txindex) {
				ret = {value, time};
			}
			else {
				out << s << ' ' << t << ' ' << value << ' ' << time << '\n';
			}
		}
		in.close();
		out.close();
		system("mv info/temp info/utxo_cold");
	}

	john.push_back({ret.first, timestamp - ret.second, block_height});
	return ret;
}

void analyze() {
	ofstream out;
	out.open("analyzed/week" + to_string(week));
	out << start_block_height << ' ' << block_height - 1 << '\n';
	out << total_tx_count - prev_total_tx_count << '\n';

	out << utxo_sum << '\n';
	out << utxo_3year_sum << '\n';
	out << utxo_5year_sum << '\n';
	out << utxo_10year_sum << '\n';

	out << utxo.size() << '\n';
	out << utxo_3year.size() << '\n';
	out << utxo_5year.size() << '\n';
	out << utxo_cnt - utxo_5year.size() - utxo_3year.size() - utxo.size() << '\n';

	out << burned.size() << '\n';
	for (int i = 0; i < burned.size(); ++i) {
		out << burned[i].first << ' ' << burned[i].second << '\n';
	}

	out << john.size() << '\n';
	for (int i = 0; i < john.size(); ++i) {
		out << get<0>(john[i]) << ' ' << get<1>(john[i]) << ' ' << get<2>(john[i]) << '\n';
	}
	out.close();

	burned.clear();
	john.clear();

	start_block_height = block_height;
	prev_total_tx_count = total_tx_count;
}

string double_sha256(unsigned char* buffer, int sz) {
	string hash = sha256(buffer, sz);
	char temp[32] = {};
	for (int i = 0; i < hash.size(); ++i) {
		if (i & 1) {
			if (hash[i] >= 'a') temp[i / 2] += (hash[i] - 'a' + 10);
			else temp[i / 2] += (hash[i] - '0');
		}
		else {
			if (hash[i] >= 'a') temp[i / 2] += (hash[i] - 'a' + 10) << 4;
			else temp[i / 2] += (hash[i] - '0') << 4;
		}
	}
	return sha256(temp, 32);
}

string reverse_hash(string hash) {
	string ret;
	for (int i = 62; i >= 0; i -= 2) {
		ret += hash[i];
		ret += hash[i + 1];
	}
	return ret;
}

string hex(unsigned char* buffer, int sz) {
	char ret[2 * sz + 1] = {};
	for (int i = 0; i < sz; ++i) {
		sprintf(ret + 2 * i, "%02x", buffer[i]);
	}
	return string(ret);
}

uint64_t VarInt(FILE* f) {
	uint64_t ret;
	fread(block + index, 1, 1, f);
	if (block[index] < 0xfd) {
		ret = block[index];
		index += 1;
	}
	else {
		if (block[index] == 0xfd) {
			index += 1;
			fread(block + index, 1, 2, f);
			ret = *((uint16_t*)(block + index));
			index += 2;
		}
		else if (block[index] == 0xfe) {
			index += 1;
			fread(block + index, 1, 4, f);
			ret = *((uint32_t*)(block + index));
			index += 4;
		}
		else {
			index += 1;
			fread(block + index, 1, 8, f);
			ret = *((uint64_t*)(block + index));
			index += 8;
		}
	}
	return ret;
}

int main() {
	restore();

	while (1) {
		FILE* f = fopen((string("blocks/") + prev_hash).c_str(), "rb");
		if (!f) {
			cerr << "File not exist!\n";
			save();
			exit(0);
		}
		index = 0;

		int sz = fread(block + index, 1, 80, f);
		string hash = reverse_hash(double_sha256(block, 80));
		string prev_hash_in_block = reverse_hash(hex(block + 4, 32));
		uint32_t timestamp = *(uint32_t*)(block + 68);
		index += sz;

		if (timestamp >= start_of_something_new + 7 * 24 * 60 * 60) {
			analyze();
			week++;
		}

		printf("%d: %s (%d)\n", block_height, hash.c_str(), timestamp);
		if (prev_hash != prev_hash_in_block) {
			cerr << "Hash mismatch!\n";
			save();
			exit(0);
		}

		int tx_count = VarInt(f);
		total_tx_count += tx_count;
//		cout << "Tx count: " << tx_count << '\n';
		if (tx_count == 0) {
			cerr << "Maybe Existence of SEGWIT!\n";
			save();
			exit(0);
		}

		uint64_t input_total = 0;
		uint64_t output_total = 0;

		for (int i = 0; i < tx_count; ++i) {
			int start = index, end;
			sz = fread(block + index, 1, 4, f);
			index += sz;

			int in_count = VarInt(f);
//			cout << "In count: " << in_count << '\n';
			for (int j = 0; j < in_count; ++j) {
				sz = fread(block + index, 1, 32, f);
				string prev_txhash = reverse_hash(hex(block + index, 32));
				index += sz;

				sz = fread(block + index, 1, 4, f);
				uint32_t prev_txindex = *(uint32_t*)(block + index);
				index += sz;
//				cout << "Prev tx info: " << prev_txhash << ' ' << prev_txindex << '\n';

				int script_length = VarInt(f);

				sz = fread(block + index, 1, script_length, f);
				index += sz;

				sz = fread(block + index, 1, 4, f);
				index += sz;

				if (prev_txhash != coinbase) {
					pair<string, uint32_t> info = {prev_txhash, prev_txindex};
					if (utxo.count(info)) {
						utxo_sum -= utxo[info];
						input_total += utxo[info];
						utxo.erase(info);
					}
					else if (utxo_3year.count(info)) {
						utxo_3year_sum -= utxo_3year[info];
						input_total += utxo_3year[info];
						utxo_3year.erase(info);
					}
					else if (utxo_5year.count(info)) {
						utxo_5year_sum -= utxo_5year[info];
						input_total += utxo_5year[info];
						utxo_5year.erase(info);
					}
					else {
						uint64_t value = erase_in_cold(prev_txhash, prev_txindex, timestamp).first;
						input_total += value;
						utxo_10year_sum -= value;
					}
					utxo_cnt--;
				}
			}

			int out_count = VarInt(f);
//			cout << "Out count: " << out_count << '\n';
			vector<uint64_t> value_collection;
			for (int j = 0; j < out_count; ++j) {
				sz = fread(block + index, 1, 8, f);
				uint64_t value = *(uint64_t*)(block + index);
				index += sz;
				output_total += value;
				value_collection.push_back(value);
//				cout << "Value: " << value << '\n';

				int script_length = VarInt(f);

				sz = fread(block + index, 1, script_length, f);
				index += sz;

				utxo_cnt++;
			}
			sz = fread(block + index, 1, 4, f);
			uint64_t lock_time = *(uint64_t*)(block + index);
			index += sz;

			end = index;

			string tx_hash = reverse_hash(double_sha256(block + start, end - start));
			for (int j = 0; j < value_collection.size(); ++j) {
				utxo[{tx_hash, j}] = value_collection[j];
				utxo_sum += value_collection[j];
				recent.push_back({timestamp, tx_hash, j});
			}
//			cout << "Tx hash: " << tx_hash << '\n';
		}
//		cout << '\n';

		fclose(f);

		// block done.. do somthing..
		system((string("mv blocks/") + prev_hash + string("trash/")).c_str());
		
		// utxo time update
		while (!recent.empty() && get<0>(*recent.begin()) <= timestamp - 3 * 365 * 24 * 60 * 60) {
			pair<string, uint32_t> info = {get<1>(*recent.begin()), get<2>(*recent.begin())};
			if (utxo.count(info)) {
				uint64_t value = utxo[info];
				utxo_3year[info] = value;
				utxo_3year_sum += value;
				utxo.erase(info);
				utxo_sum -= value;
				recent_3year.push_back(*recent.begin());
			}
			recent.pop_front();
		}
		while (!recent_3year.empty() && get<0>(*recent_3year.begin()) <= timestamp - 5 * 365 * 24 * 60 * 60) {
			pair<string, uint32_t> info = {get<1>(*recent_3year.begin()), get<2>(*recent_3year.begin())};
			if (utxo_3year.count(info)) {
				uint64_t value = utxo_3year[info];
				utxo_5year[info] = value;
				utxo_5year_sum += value;
				utxo_3year.erase(info);
				utxo_3year_sum -= value;
				recent_5year.push_back(*recent_3year.begin());
			}
			recent_3year.pop_front();
		}
		while (!recent_5year.empty() && get<0>(*recent_5year.begin()) <= timestamp - 10 * 365 * 24 * 60 * 60) {
			pair<string, uint32_t> info = {get<1>(*recent_5year.begin()), get<2>(*recent_5year.begin())};
			if (utxo_5year.count(info)) {
				uint64_t value = utxo_5year[info];
				utxo_10year[info] = {value, get<0>(*recent_5year.begin())};
				utxo_10year_sum += value;
				if (utxo_10year.size() > MAX_COLD_SIZE) {
					flush();
				}
				utxo_5year.erase(info);
				utxo_5year_sum -= value;
			}
			recent_5year.pop_front();
		}

		if (output_total - input_total != reward[block_height / 210000]) {
			cerr << reward[block_height / 210000] - (output_total - input_total) << "Satoshi permanently disappered on block " << block_height << '\n';
			burned.push_back({reward[block_height / 210000] - (output_total - input_total), block_height});
		}

		prev_hash = hash;
		block_height++;
	}
}
