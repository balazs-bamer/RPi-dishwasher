#ifndef DISHWASHER_BANCOPYMOVE_H_INCLUDED
#define DISHWASHER_BANCOPYMOVE_H_INCLUDED

/// Class to inhibit copy or move construction or initialization of the subclasses.
class BanCopyMove {
public:
	BanCopyMove() = default;
	BanCopyMove(const BanCopyMove &other) = delete;
	BanCopyMove(BanCopyMove &&other) = delete;
	BanCopyMove &operator=(const BanCopyMove &other) = delete;
	BanCopyMove &operator=(BanCopyMove &&other) = delete;
};


#endif // BANCOPYMOVE_H_INCLUDED
