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

int64_t utxo_10year_sum;
int utxo_10year_cnt;

int total_tx_count;
int prev_total_tx_count;
int start_block_height;

// utxo: (txhash, txindex): (value, timestamp)
unordered_map<pair<string, uint32_t>, pair<uint64_t, uint32_t>, pair_hash> utxo;

void restore() {
	ifstream in;
	in.open("info/basic");
	in >> prev_hash;
	in >> block_height;
	in >> week;

	int sz;
	in >> sz;
	for (int i = 0; i < sz; ++i) {
		uint64_t a;
		int b;
		in >> a >> b;
		burned.push_back({a, b});
	}

	in >> sz;
	for (int i = 0; i < sz; ++i) {
		uint64_t a;
		uint32_t b;
		int c;
		in >> a >> b >> c;
		john.push_back({a, b, c});
	}

	in >> utxo_10year_sum;
	in >> utxo_10year_cnt;

	in >> total_tx_count;
	in >> prev_total_tx_count;
	in >> start_block_height;
	in.close();

	string s;
	uint32_t t, time;
	uint64_t value;

	in.open("info/utxo");
	while (in >> s >> t >> value >> time) utxo[{s, t}] = {value, time};
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

	out << utxo_10year_sum << '\n';
	out << utxo_10year_cnt << '\n';

	out << total_tx_count << '\n';
	out << prev_total_tx_count << '\n';
	out << start_block_height << '\n';
	out.close();

	out.open("info/utxo");
	for (auto it = utxo.begin(); it != utxo.end(); ++it) {
		out << it->first.first << ' ' << it->first.second << ' ';
		out << it->second.first << ' ' << it->second.second << '\n';
	}
	out.close();
}

pair<uint64_t, uint32_t> erase_in_cold(string txhash, uint32_t txindex) {
	pair<uint64_t, uint32_t> ret;
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

	utxo_10year_sum -= ret.first;
	utxo_10year_cnt--;

	return ret;
}

void analyze(uint32_t timestamp) {
	int64_t utxo_sum = 0;
	int64_t utxo_3year_sum = 0;
	int64_t utxo_5year_sum = 0;

	int utxo_cnt = 0;
	int utxo_3year_cnt = 0;
	int utxo_5year_cnt = 0;

	vector<pair<string, uint32_t>> cold;
	for (auto it = utxo.begin(); it != utxo.end(); ++it) {
		if (it->second.second > timestamp - 3 * 365 * 24 * 60 * 60) {
			utxo_sum += it->second.first;
			utxo_cnt++;
		}
		else if (it->second.second > timestamp - 5 * 365 * 24 * 60 * 60) {
			utxo_3year_sum += it->second.first;
			utxo_3year_cnt++;
		}
		else if (it->second.second > timestamp - 10 * 365 * 24 * 60 * 60) {
			utxo_5year_sum += it->second.first;
			utxo_5year_cnt++;
		}
		else {
			cold.push_back(it->first);
		}
	}

	ofstream out;
	out.open("info/utxo_cold", fstream::out | fstream::app);
	for (auto it = cold.begin(); it != cold.end(); ++it) {
		out << it->first << ' ' << it->second << ' ';
		out << utxo[*it].first << ' ' << utxo[*it].second << '\n';
		utxo_10year_sum += utxo[*it].first;
		utxo_10year_cnt++;
		utxo.erase(*it);
	}
	out.close();

	out.open("analyzed_temp/week" + to_string(week));
	out << start_block_height << ' ' << block_height - 1 << '\n';
	out << total_tx_count - prev_total_tx_count << '\n';

	out << utxo_sum << '\n';
	out << utxo_3year_sum << '\n';
	out << utxo_5year_sum << '\n';
	out << utxo_10year_sum << '\n';

	out << utxo_cnt << '\n';
	out << utxo_3year_cnt << '\n';
	out << utxo_5year_cnt << '\n';
	out << utxo_10year_cnt << '\n';

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

uint64_t VarInt(FILE* f, bool increase) {
	uint64_t ret;
	fread(block + index, 1, 1, f);
	if (block[index] < 0xfd) {
		ret = block[index];
		if (increase) index += 1;
	}
	else {
		if (block[index] == 0xfd) {
			if (increase) index += 1;
			fread(block + index, 1, 2, f);
			ret = *((uint16_t*)(block + index));
			if (increase) index += 2;
		}
		else if (block[index] == 0xfe) {
			if (increase) index += 1;
			fread(block + index, 1, 4, f);
			ret = *((uint32_t*)(block + index));
			if (increase) index += 4;
		}
		else {
			if (increase) index += 1;
			fread(block + index, 1, 8, f);
			ret = *((uint64_t*)(block + index));
			if (increase) index += 8;
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

		if (timestamp >= start_of_something_new + week * 7 * 24 * 60 * 60) {
			analyze(start_of_something_new + week * 7 * 24 * 60 * 60);
			week++;
		}

		printf("%d: %s (%d)\n", block_height, hash.c_str(), timestamp);
		if (prev_hash != prev_hash_in_block) {
			cerr << "Hash mismatch!\n";
			save();
			exit(0);
		}

		int tx_count = VarInt(f, 1);
		total_tx_count += tx_count;
//		cout << "Tx count: " << tx_count << '\n';

		uint64_t input_total = 0;
		uint64_t output_total = 0;

		for (int i = 0; i < tx_count; ++i) {
			int start = index, end;
			sz = fread(block + index, 1, 4, f);
			index += sz;

			bool segwit = 0;
			int in_count = VarInt(f, 1);
			if (in_count == 0) { // SEGWIT
				segwit = 1;

				fread(block + index, 1, 1, f);
				index--;

				in_count = VarInt(f, 1);
			}
//			cout << "In count: " << in_count << '\n';
			for (int j = 0; j < in_count; ++j) {
				sz = fread(block + index, 1, 32, f);
				string prev_txhash = reverse_hash(hex(block + index, 32));
				index += sz;

				sz = fread(block + index, 1, 4, f);
				uint32_t prev_txindex = *(uint32_t*)(block + index);
				index += sz;
//				cout << "Prev tx info: " << prev_txhash << ' ' << prev_txindex << '\n';

				int script_length = VarInt(f, 1);

				sz = fread(block + index, 1, script_length, f);
				index += sz;

				sz = fread(block + index, 1, 4, f);
				index += sz;

				if (prev_txhash != coinbase) {
					pair<string, uint32_t> info = {prev_txhash, prev_txindex};
					pair<uint64_t, uint32_t> value;
					if (utxo.count(info)) {
						value = utxo[info];
						utxo.erase(info);
					}
					else {
						value = erase_in_cold(prev_txhash, prev_txindex);
					}
					input_total += value.first;
					if (value.second <= timestamp - 10 * 365 * 24 * 60 * 60) {
						john.push_back({value.first, timestamp - value.second, block_height});
					}
				}
			}

			int out_count = VarInt(f, 1);
//			cout << "Out count: " << out_count << '\n';
			vector<uint64_t> value_collection;
			for (int j = 0; j < out_count; ++j) {
				sz = fread(block + index, 1, 8, f);
				uint64_t value = *(uint64_t*)(block + index);
				index += sz;
				output_total += value;
				value_collection.push_back(value);
//				cout << "Value: " << value << '\n';

				int script_length = VarInt(f, 1);

				sz = fread(block + index, 1, script_length, f);
				index += sz;
			}

			if (segwit) {
				for (int j = 0; j < in_count; ++j) {
					int segwit_cnt = VarInt(f, 0);
					for (int k = 0; k < segwit_cnt; ++k) {
						int segwit_length = VarInt(f, 0);
						fread(block + index, 1, segwit_length, f);
					}
				}
			}

			sz = fread(block + index, 1, 4, f);
			uint64_t lock_time = *(uint64_t*)(block + index);
			index += sz;

			end = index;

			string tx_hash = reverse_hash(double_sha256(block + start, end - start));
			for (int j = 0; j < value_collection.size(); ++j) {
				if (utxo.count({tx_hash, j})) {
					utxo[{tx_hash, j}].first += value_collection[j];
					utxo[{tx_hash, j}].second = timestamp;
				}
				else {
					utxo[{tx_hash, j}] = {value_collection[j], timestamp};
				}
			}
//			cout << "Tx hash: " << tx_hash << '\n';
		}
//		cout << '\n';

		fclose(f);

		// block done.. do somthing..
		system((string("mv blocks/") + prev_hash + string(" trash/")).c_str());
		
		// utxo time update

		if (output_total - input_total != reward[block_height / 210000]) {
//			cerr << reward[block_height / 210000] - (output_total - input_total) << "Satoshi permanently disappered on block " << block_height << '\n';
			burned.push_back({reward[block_height / 210000] - (output_total - input_total), block_height});
		}

		prev_hash = hash;
		block_height++;
	}
}
