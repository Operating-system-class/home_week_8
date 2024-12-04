#include <iostream>
#include <bitset>
#include <cstring>
#include <list>

using namespace std;

const int OFFSET_BITS = 3;
const int PAGE_NUM_BITS = 5;
const int FRAME_NUM_BITS = 13;
const int PAGE_TABLE_SIZE = 1 << PAGE_NUM_BITS;
const int TLB_SIZE = 4;

typedef bitset<PAGE_NUM_BITS> PageNumber;
typedef bitset<FRAME_NUM_BITS> FrameNumber;
typedef bitset<OFFSET_BITS> OffsetNumber;
typedef bitset<PAGE_NUM_BITS + OFFSET_BITS> LogicalAddress;
typedef bitset<FRAME_NUM_BITS + OFFSET_BITS> PhysicalAddress;


// the page table
FrameNumber PT[PAGE_TABLE_SIZE];

// the TLB
pair<PageNumber, FrameNumber> TLB[TLB_SIZE];
int lastTLBEntry = 0;


// Returns physical address in `pa` and whether the TLB is hit in `TLBhit`
// Trả về địa chỉ vật lý vào biến `pa` và có sử dụng TLB hay không trong biết `TLBhit`
void MMU_translate(LogicalAddress la, PhysicalAddress& pa, bool& TLBhit) {
    // Extract page number and offset from logical address
    PageNumber pageNumber;
    OffsetNumber offset;
    for (int i = 0; i < PAGE_NUM_BITS; i++) {
        pageNumber[i] = la[i + OFFSET_BITS];
    }
    for (int i = 0; i < OFFSET_BITS; i++) {
        offset[i] = la[i];
    }

    // Check TLB for the page number
    TLBhit = false;
    FrameNumber frameNumber;
    for (int i = 0; i < TLB_SIZE; i++) {
        if (TLB[i].first == pageNumber) {
            frameNumber = TLB[i].second;
            TLBhit = true;
            break;
        }
    }

    // If TLB miss, get frame number from page table and update TLB
    if (!TLBhit) {
        frameNumber = PT[pageNumber.to_ulong()];
        TLB[lastTLBEntry] = make_pair(pageNumber, frameNumber);
        lastTLBEntry = (lastTLBEntry + 1) % TLB_SIZE;
    }

    // Combine frame number and offset to form physical address
    for (int i = 0; i < OFFSET_BITS; i++) {
        pa[i] = offset[i];
    }
    for (int i = 0; i < FRAME_NUM_BITS; i++) {
        pa[i + OFFSET_BITS] = frameNumber[i];
    }
}

int main(int argc, const char** argv) {
	list<LogicalAddress> accessList;

	if (argc <= 1 || strcmp(argv[1], "-i") != 0) {
		for (int i = 0; i < PAGE_TABLE_SIZE; i++)
			PT[i] = (i + 37) * (i + 3) + 231;

		for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
			int n = ((i + 13) * (i + 7) + 891) % PAGE_TABLE_SIZE,
				m = ((i + 21) * (i + 17) + 533) % PAGE_TABLE_SIZE;
			auto t = PT[m];
			PT[m] = PT[n];
			PT[n] = t;
		}

		srand(47261);
		for (int i = 0; i < PAGE_TABLE_SIZE * 5; i++) {
			int a = (rand() % PAGE_TABLE_SIZE) << OFFSET_BITS;
			int n = rand() % 20;
			for (int j = 0; j < n; j++)
				accessList.push_back(LogicalAddress(a + rand() % (1 << OFFSET_BITS)));
		}

	} else {
		for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
			int f;
			cin >> f;
			PT[i] = f;
		}

		int n;
		cin >> n;

		for (int i = 0; i < n; i++) {
			LogicalAddress la;
			cin >> la;
			accessList.push_back(la);
		}
	}

	for (int i = 0; i < TLB_SIZE; i++)
		TLB[i] = make_pair(PageNumber(i), PT[i]);

	int TLBhitcount = 0;
	for (auto la : accessList) {
		PhysicalAddress pa;
		bool TLBhit;
		MMU_translate(la, pa, TLBhit);

		if (TLBhit) TLBhitcount++;

		cout << la << " --> " << pa << " " << TLBhit << endl;
	}

	cout << "TLB hit rate: " << (TLBhitcount * 100. / accessList.size()) << "%" << endl;

	return 0;
}