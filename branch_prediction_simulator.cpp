#include <cstdlib>
#include <cstdint>


#include <fstream>
#include <exception>
#include <iomanip>
#include <iostream>
#include <forward_list>
#include <stdexcept>
#include <string>
#include <vector>


enum PredictionOutcome
{
	NotTaken,
	Taken
};

unsigned pow2(std::uint8_t x)
{
	if (x > 32)
	{
		throw std::invalid_argument("The argument of pow2() is greater than 32.");
	}

	unsigned y = 1;
	while (x != 0)
	{
		y <<= 1;
		x -= 1;
	}

	return y;
}


// class BitArray final
// {

// public:
// 	BitArray() = default;
// 	~BitArray() = default;

// 	explicit BitArray(unsigned value);
// 	BitArray(std::uint8_t length, bool value);

// 	operator unsigned() const { return _bits; }
// 	std::uint8_t getLength() const { return _length; }
// 	void setLength(std::uint8_t length);

// 	void set(std::uint8_t pos, bool val);

// 	BitArray operator<<(std::uint8_t n) const;
// 	BitArray& operator<<=(std::uint8_t n);
// 	BitArray operator>>(std::uint8_t n) const;
// 	BitArray& operator>>=(std::uint8_t n);

// private:
// 	unsigned _bits = 0;
// 	std::uint8_t _length = 0;

// 	friend BitArray operator&(const BitArray& lhs, const BitArray& rhs);
// 	friend BitArray operator|(const BitArray& lhs, const BitArray& rhs);
// 	friend BitArray operator^(const BitArray& lhs, const BitArray& rhs);
// };


class BranchPredictor
{
public:
	BranchPredictor(const BranchPredictor&) = delete;
	BranchPredictor& operator=(const BranchPredictor&) = delete;

protected:
	BranchPredictor() = default;	

public:
	virtual ~BranchPredictor() = default;

public:
	double getMissPredictionRatio() const;

	virtual void update(unsigned address, PredictionOutcome predOutcome) = 0;

protected:
	unsigned missCount = 0;
	unsigned totalCount = 0;
};

class GshareBranchPredictor final : public BranchPredictor
{
public:
	GshareBranchPredictor(unsigned gbtBitsCount, unsigned ghrBitsCount);
	~GshareBranchPredictor() override = default;

public:
	void update(unsigned address, PredictionOutcome predOutcome) override;

private:
	std::uint8_t getBufferTableEntry(std::size_t row, std::size_t col) const;
	void setBufferTableEntry(std::size_t row, std::size_t col, std::uint8_t val);

private:
	unsigned gbtBitsCount = 0;
	unsigned ghrBitsCount = 0;
	unsigned gbtBitsMask = 0;
	unsigned ghrBitsMask = 0;
	std::vector<std::uint8_t> gbt;	// Global Buffer Table (GBT)
//	BitArray ghr; // Global History Record (GBR)
	unsigned ghr = 0; // Global History Record (GBR)
};

std::forward_list<std::string> readLines(std::istream& is)
{
	std::forward_list<std::string> lines;

	while (!is.eof())
	{
		std::string line;
		// std::getline(line, is);
		std::getline(is, line);
		if (line.empty())
		{
			continue;
		}
		lines.emplace_front(line);
	}

	lines.reverse();

	return lines;
}

int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		std::cerr << "Usage: " << argv[0] << "<GBT bits count> <GHR bits count> <input file>\n";
		std::exit(EXIT_FAILURE);
	}

	unsigned gbtBitsCount = 0, ghrBitsCount = 0;
	try
	{
		gbtBitsCount = std::stoul(argv[1], NULL, 10);
		ghrBitsCount = std::stoul(argv[2], NULL, 10);
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Exception (when parsing program arguments): '" << ex.what() << "'\n";
		std::exit(EXIT_FAILURE);
	}


	if (!(gbtBitsCount >= 16 && gbtBitsCount <= 32))
	{
		std::cerr << "GBT bits count must be in range 16 - 32!\n";
		std::exit(EXIT_FAILURE);
	}

	if (ghrBitsCount > 32)
	{
		std::cerr << "GHR bits count must be in range 0 - 32!\n";
		std::exit(EXIT_FAILURE);
	}

	if (!(gbtBitsCount >= ghrBitsCount))
	{
		std::cerr << "GBT bits count must be greater than GHR bits count!\n";
		std::exit(EXIT_FAILURE);
	}


	// read the data from the input file
	std::forward_list<std::string> lines;

	{
		const char* inFileName = argv[3];
		std::ifstream inFile(inFileName);

		if (!inFile.good())
		{
			std::cerr << "Could not open file: '" << inFileName << "'\n";
			std::exit(EXIT_FAILURE);
		}

	 	lines = readLines(inFile);
	}


	GshareBranchPredictor branchPredictor(gbtBitsCount, ghrBitsCount);

	for (const std::string& line : lines)
	{
		if (line.empty())
		{
			continue;
		}

		size_t p = line.find(' ');
		if (p == std::string::npos || (++p) == line.length())
		{
			std::cerr << "Invalid line: '" << line << "'\n";
			continue;
		}

		PredictionOutcome predOutcome;
		if (line[p] == 'n')
		{
			predOutcome = NotTaken;
		}
		else if (line[p] == 't')
		{
			predOutcome = Taken;
		}
		else
		{
			std::cerr << "Invalid prediction outcome code: '" << line[p] << "'\n";
			continue;
		}

		unsigned address = 0L;

		try 
		{
			address = std::stoul(line, NULL, 16);
		} 
		catch(const std::exception& ex)
		{
			std::cerr << "Exception: '" << ex.what() << "'\n";
			continue;
		}

		branchPredictor.update(address, predOutcome);
	}


	std::cout << "GBT bits count: " << gbtBitsCount
			<< "\tGHR bits count: " << ghrBitsCount
			<< "\tmisprediction ratio: " << std::setprecision(2)
			<< branchPredictor.getMissPredictionRatio() << std::endl;


	std::exit(EXIT_SUCCESS);
	return 0;
}


// ////////////////////////////////////////////////////////////////////////////////////////
// // Implementation of BitArray
// BitArray::BitArray(unsigned value)
// 	: _bits(value)
// {

// }

// BitArray::BitArray(std::uint8_t length, bool value)
// 	: _length(length)
// {
// 	// TO DO: fill length bits with value
// }


// void BitArray::setLength(std::uint8_t length)
// {

// }

// void BitArray::set(std::uint8_t pos, bool val)
// {

// }

// BitArray BitArray::operator<<(std::uint8_t n) const
// {
// 	return BitArray();
// }

// BitArray& BitArray::operator<<=(std::uint8_t n)
// {
// 	return *this;
// }

// BitArray BitArray::operator>>(std::uint8_t n) const
// {
// 	return BitArray();
// }

// BitArray& BitArray::operator>>=(std::uint8_t n)
// {
// 	return *this;
// }


// BitArray operator&(const BitArray& lhs, const BitArray& rhs)
// {
// 	return BitArray();
// }

// BitArray operator|(const BitArray& lhs, const BitArray& rhs)
// {
// 	return BitArray();
// }

// BitArray operator^(const BitArray& lhs, const BitArray& rhs)
// {
// 	return BitArray();
// }


////////////////////////////////////////////////////////////////////////////////////////
// Implementation of BranchPredictor
double BranchPredictor::getMissPredictionRatio() const
{
	if (totalCount == 0)
	{
		return 0.0;
	}

	double ratio = static_cast<double>(missCount * 100) / static_cast<double>(totalCount);
	return ratio;
}

////////////////////////////////////////////////////////////////////////////////////////
// Implementation of GshareBranchPredictor
GshareBranchPredictor::GshareBranchPredictor(unsigned gbtBitsCount, unsigned ghrBitsCount)
	: gbtBitsCount(gbtBitsCount)
	, ghrBitsCount(ghrBitsCount)
{
	for (unsigned i = 0; i < gbtBitsCount; i++)
	{
		gbtBitsMask |= 1 << i;
	}

	unsigned gbtLength = pow2(gbtBitsCount - 2);	// take into account that each row contains 4 2-bit counters
	// // self - test
	// gbt.resize(gbtLength, 0b11100100);
	// // for (unsigned row = 0; row < gbtLength; row++)
	// // {
	// // 	for (unsigned col = 0; col < 4; col++)
	// // 	{
	// // 		setBufferTableEntry(row, col, col);
	// // 	}
	// // }

	// unsigned faultCount = 0;
	// for (unsigned row = 0; row < gbtLength; row++)
	// {
	// 	for (unsigned col = 0; col < 4; col++)
	// 	{
	// 		std::uint8_t val = getBufferTableEntry(row, col);
	// 		if (val != col)
	// 		{
	// 			std::cerr << "failure: row = " 
	// 				<< row << " col = " << col << " val = " << val
	// 				<< std::endl;
	// 			faultCount++;
	// 		}
	// 	}
	// }

	// if (faultCount != 0)
	// {
	// 	std::cout << "Self test failed!\n";
	// }
	// else
	// {
	// 	std::cout << "Self test passed!\n";
	// }

	gbt.resize(gbtLength, 0b10101010);	// initialize all GBT entries to 'Weakly Taken' (2)
}

void GshareBranchPredictor::update(unsigned address, PredictionOutcome predOutcome)
{
	address &= gbtBitsMask;

	// take into account GHR
	if (ghrBitsCount)
	{
		unsigned ghrCopy = ghr & gbtBitsMask;
		ghrCopy <<= (gbtBitsCount - ghrBitsCount);
		ghrCopy &= gbtBitsMask;
		address ^= ghrCopy;
	}


	unsigned row = address / 4;
	unsigned col = address % 4;
	std::uint8_t cnt = getBufferTableEntry(row, col);

	if (predOutcome == NotTaken)
	{
		if (cnt >= 2)
		{
			missCount++;
		}

		if (cnt > 0)	// ensure the smalest value is 'Strongly Not Taken'
		{
			cnt--;
		}		

	}
	else if (predOutcome == Taken)
	{
		if (cnt <= 1)
		{
			missCount++;
		}

		if (cnt < 3)	// ensure the largest value is 'Strongly Taken'
		{
			cnt++;
		}
	}

	setBufferTableEntry(row, col, cnt);

	if (ghrBitsCount)
	{
		// update GHR
		ghr >>= 1;
		if (predOutcome == Taken)
		{
			ghr |= (1 << (ghrBitsCount - 1));
		}
	}

	totalCount++;
}

std::uint8_t GshareBranchPredictor::getBufferTableEntry(std::size_t row, std::size_t col) const
{
	if (row >= gbt.size())
	{
		throw std::out_of_range("Row index is too large to get value of GBT.");
	}

	if (col >= 4)
	{
		throw std::out_of_range("Column index is too large to get value of GBT.");
	}

	std::uint8_t val = gbt[row] >> (col * 2);
	return val & 0b11;
}

void GshareBranchPredictor::setBufferTableEntry(std::size_t row, std::size_t col, std::uint8_t val)
{
	if (row >= gbt.size())
	{
		throw std::out_of_range("Row index is too large to set value of GBT.");
	}

	if (val > 3)
	{
		throw std::invalid_argument("The value is too large to update entry of GBT.");
	}

	switch (col)
	{
	case 0:
		gbt[row] &= 0b11111100;
		gbt[row] |= val & 0b11;
		break;
	case 1:
		gbt[row] &= 0b11110011;
		gbt[row] |= (val << 2) & 0b1100;
		break;
	case 2:
		gbt[row] &= 0b11001111;
		gbt[row] |= (val << 4) & 0b110000;
		break;
	case 3:
		gbt[row] &= 0b00111111;
		gbt[row] |= (val << 6) & 0b11000000;
		break;
	default:
		throw std::out_of_range("Column index is too large to set value of GBT.");
	}
}
