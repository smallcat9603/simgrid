/*
 * Matrix.h
 *
 *  Created on: 26 avr. 2012
 *      Author: chaix
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include <vector>
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <sstream>

template<typename T> T mymax(T a, T b) {if (a > b) return(a); else return(b);};
template<typename T> T mymin(T a, T b) {if (a < b) return(a); else return(b);};
template<typename T> void myswap(T& a, T& b) {T temp = a; a = b; b = temp;}


template<typename T> class Matrix
{
public:

	//if set to true, accessing to element (i,j) or (j,i) will reach the same element
	bool mirrorMode;

	Matrix<T>():mirrorMode(false),data(NULL),dataSize(0),NRows(0),NCols(0){}
	Matrix<T>(unsigned int NRows, unsigned int NCols=1,T initValue=0,bool mirrorMode=false):mirrorMode(mirrorMode),data(NULL),dataSize(0),NRows(NRows),NCols(NCols)
	{
		allocateData(NRows*NCols);
		for (unsigned int i = 0; i < NRows*NCols; ++i) {
			assert(i<dataSize);
			data[i]=initValue;
		}
	}
	Matrix<T>(const Matrix<T>& old):data(NULL),dataSize(0){copy(old);}
	virtual ~Matrix<T>(){clear();}
	Matrix<T>& operator=(const Matrix<T>& old){copy(old);return *this;}
	void clear()
	{
		NCols=0;
		NRows=0;
		if(data!=NULL)
			delete[] data;
		data=NULL;
		dataSize=0;
	}
	void reset(T defaultValue=0)
	{
		const unsigned int NElements=NRows*NCols;
		for (unsigned int i = 0; i < NElements; ++i) {
				assert(i<dataSize);
				data[i]=defaultValue;
		}
	}


	static Matrix<T> buildUniformMatrix(unsigned int NRows, unsigned int NCols, T MaxValue)
	{
		Matrix<T> data(NRows,NCols);
		for (unsigned int r = 0; r < NRows; ++r) {
			for (unsigned int c = 0; c < NCols; ++c) {
				data(r,c)=(T)std::fmod((double)rand(),(double)MaxValue);
			}
		}
		return data;
	}

	static Matrix<T> buildPermutationVector(unsigned int NRows)
	{
		Matrix<T> data(NRows,1);
		for (unsigned int r = 0; r < NRows; ++r)
			data(r)=r;
		for (unsigned int r = 0; r < NRows; ++r)
			myswap(data(r),data(rand()%NRows));
		return data;
	}



	Matrix<T> operator*(const Matrix<T>& b)
	{
		Matrix<T> result(getRows(),b.getCols());
		assert(getCols()==b.getRows());
		for (unsigned int r = 0; r < getRows(); ++r) {
			for (unsigned int c = 0; c < b.getCols(); ++c) {
				result(r,c)=0;
				for (unsigned int i = 0; i < getCols(); ++i) {
					result(r,c)+=(*this)(r,i)*b(i,c);
				}
			}
		}
		return result;
	}

	Matrix<T> operator*(T b)
	{
		Matrix<T> result=*this;
		for (unsigned int r = 0; r < getRows(); ++r) {
			for (unsigned int c = 0; c < getCols(); ++c) {
				result(r,c)*=b;
			}
		}
		return result;
	}


	Matrix<T> operator+(const Matrix<T>& b)
	{
		Matrix<T> result(getRows(),getCols());
		assert(getCols()==b.getCols());
		assert(getRows()==b.getRows());
		for (unsigned int r = 0; r < getRows(); ++r) {
			for (unsigned int c = 0; c < b.getCols(); ++c) {
				result(r,c)=(*this)(r,c)+b(r,c);
				}
			}
		return result;
	}




	Matrix<T> operator-(const Matrix<T>& b) const
	{
		Matrix<T> result(getRows(),getCols());
		assert(getCols()==b.getCols());
		assert(getRows()==b.getRows());
		for (unsigned int r = 0; r < getRows(); ++r) {
			for (unsigned int c = 0; c < b.getCols(); ++c) {
				result(r,c)=(*this)(r,c)-b(r,c);
				}
			}
		return result;
	}
	
	bool operator==(const Matrix<T>& b) const
	{
		if(getCols()!=b.getCols() || getRows()!=b.getRows())
			return false;

		for (unsigned int r = 0; r < getRows(); ++r) {
			for (unsigned int c = 0; c < b.getCols(); ++c) {
				if( (*this)(r,c)!=b(r,c))
					return false;
				}
			}
		return true;
	}

	Matrix<T>& operator+=(const Matrix<T>& b)
	{
		assert(getCols()==b.getCols());
		assert(getRows()==b.getRows());
		for (unsigned int i = 0; i < getRows()*getCols(); ++i) {
				assert(i<dataSize);
				assert(i<b.dataSize);
				data[i]+=b.data[i];
				}
		return *this;
	}


	Matrix& maxWith(const Matrix<T>& b)
	{
		assert(getCols()==b.getCols());
		assert(getRows()==b.getRows());
		for (unsigned int r = 0; r < getRows(); ++r) {
			for (unsigned int c = 0; c < b.getCols(); ++c) {
				(*this)(r,c)=(std::max)((*this)(r,c),b(r,c));
			}
		}
		return *this;
	}

	void toVectorStream(std::ostream& out, std::string sep="\t") const
	{
		assert(NCols==1 || NRows==1);
		const unsigned int NElements=NCols*NRows;
		for (unsigned int i = 0; i < NElements; ++i) {
			assert(i<dataSize);
			out<<this->data[i];
			if(i+1<NElements)
				out<<sep;
		}
		out<<std::endl;
	}

	bool fromVectorStream(std::istream& in)
	{
		NCols=0;
		NRows=0;
		//Raah..
		std::vector<T> tmp;
		T t;
		//Remove trailing spaces and newlines
		while(!isAlphaNum(in.peek()))
		{
			in.get();
			if(!in.good())
				return false;
		}
		//Read the line
		while(in.peek()!='\n')
		{
			if(!in.good())
				return false;
			in>>t;tmp.push_back(t);
		}
		if(!in.good())
			return false;

		NCols=1;
		NRows=tmp.size();
		const unsigned int NElements=NRows;
		allocateData(NElements);
		//Copy back the values from vector
		for (unsigned int i = 0; i < NRows; ++i)
			this->data[i]=tmp[i];
		return true;
	}

	void toSquareStream(std::ostream& out, std::string sep="\t") const
	{
		assert(NCols==NRows);
		const unsigned int NElements=NRows*NRows;
		for (unsigned int i = 0; i < NElements; ++i) {
			assert(i<dataSize);
			out<<this->data[i];
			if((i+1)%NRows==0)
				out<<'\n';
			else
				out<<sep;
		}
	}

	bool fromSquareStream(std::istream& in)
	{
		//Raah..
		std::vector<T> tmp;
		T t;
		//Remove trailing spaces and newlines
		while(!isAlphaNum(in.peek()))
		{
			in.get();
			if(!in.good())
				return false;
		}

		while(in.peek()!='\n')
		{
			if(!in.good())
				return false;
			in>>t;tmp.push_back(t);
			//std::cout<<"Reading "<<t<< " Size "<< tmp.size()<<std::endl;
		}
		if(!in.good())
			return false;
		//std::cout<<"Final Size "<< tmp.size()<<std::endl;
		NCols=NRows=tmp.size();
		const unsigned int NElements=NRows*NRows;
		allocateData(NElements);
		//Copy back the values from vector
		for (unsigned int i = 0; i < NRows; ++i)
			this->data[i]=tmp[i];
		//Copy directly subsequent rows
		for (unsigned int i = NRows; i < NElements; ++i) {
			assert(i<dataSize);
			in>>this->data[i];
		}
		return true;
	}

	void toMatrixStream(std::ostream& out, char sep="\t") const
	{
		out<<NRows<<sep<<NCols<<'\n';
		const unsigned int NElements=NRows*NCols;
		for (unsigned int i = 0; i < NElements; ++i) {
			assert(i<dataSize);
			out<<this->data[i];
			if((i+1)%NRows==0)
				out<<'\n';
			else
				out<<sep;
		}
	}

	bool fromMatrixStream(std::istream& in)
	{
		//Remove trailing spaces and newlines
		while(!isAlphaNum(in.peek()))
		{
			in.get();
			if(!in.good())
				return false;
		}
		
		in>>NRows>>NCols;
		if(!in.good())
			return false;
			
		const unsigned int NElements=NRows*NCols;

		allocateData(NElements);

		for (unsigned int i = 0; i< NElements ; ++i) {
			assert(i<=dataSize);
			in>>this->data[i];
		}
		return true;
	}


	void toStream(std::ostream& os)
	{
		if(getRows()==1 || getCols()==1)
	       toVectorStream(os);
		else if(getRows()==getCols())
	       toSquareStream(os);
		else
		   toMatrixStream(os);

	}

	std::string print()
	{
		std::stringstream ss;
		toStream(ss);
		return ss.str();
	}
	
		
	T& operator()(unsigned int row, unsigned int col)
	{
		assert(row<NRows && col<NCols);
		assert(row*NCols+col<dataSize);
		if(mirrorMode)
		{
			unsigned int minimum=mymin<unsigned int>(row,col);
			unsigned int maximum=mymax<unsigned int>(row,col);
			return data[minimum*NCols+maximum];			
		}
		return data[row*NCols+col];
	}
	const T& operator()(unsigned int row, unsigned int col) const
	{
		assert(row<NRows && col<NCols);
		assert(row*NCols+col<dataSize);
		if(mirrorMode)
		{
			unsigned int minimum=mymin<unsigned int>(row,col);
			unsigned int maximum=mymax<unsigned int>(row,col);
			return data[minimum*NCols+maximum];			
		}
		return data[row*NCols+col];
	}

	T& operator()(unsigned int row)
	{
		assert(row<NRows && NCols==1);
		return data[row];
	}

	const T& operator()(unsigned int row) const
	{
		assert(row<NRows && NCols==1);
		return data[row];
	}

	unsigned int getRows() const {return NRows;}
	unsigned int getCols() const {return NCols;}

	T getNorm(unsigned int degree)
	{
		T acc=0;
		for (unsigned int i = 0; i < dataSize; ++i)
		{
			switch(degree)
			{
				case 1:
					acc+=std::abs(data[i]);break;
				case 2:
					acc+=data[i]*data[i];break;
				default:
					acc+=std::abs(std::pow(data[i],(T)degree));break;
			}	
		}
		
		
		switch(degree)
		{
			case 1:
				break;
			case 2:
				acc=std::sqrt(acc);break;
			default:
				acc=std::pow(acc,1/(T)degree);break;
		}
		return acc/(T)dataSize;
	}
	
	
	/**
	 * Return a submatrix, from start row/col(included) to end row/col (not included)
	 */
	Matrix<T> getSubMatrix(unsigned int startRow, unsigned int startCol, unsigned int endRow, unsigned int endCol)
	{
		assert(endRow>startRow && endRow<=NRows);
		assert(endCol>startCol && endCol<=NCols);
		unsigned int subRows=endRow-startRow;
		unsigned int subCols=endCol-startCol;
		Matrix<T> result(subRows,subCols);
		for (unsigned int r = 0; r < subRows; ++r) {
					for (unsigned int c = 0; c < subCols; ++c) {
						result(r,c)=(*this)(startRow+r,startCol+c);
					}
		}
		return result;
	}

	void setSubMatrix(unsigned int startRow, unsigned int startCol, const Matrix<T>& subMatrix)
	{
		assert(startRow+subMatrix.getRows()<=NRows);
		assert(startCol+subMatrix.getCols()<=NCols);
		for (unsigned int r = 0; r < subMatrix.getRows(); ++r) {
					for (unsigned int c = 0; c < subMatrix.getCols(); ++c) {
						(*this)(startRow+r,startCol+c)=subMatrix(r,c);
					}
		}
	}

	bool isSquare() const {return NRows==NCols;}
	bool isVector() const {return NCols==1;}

private:
	T* data;
	unsigned int dataSize;

	unsigned int NRows;
	unsigned int NCols;

	void allocateData(unsigned int NElements)
	{
		assert(NElements>=1);

		//If current data table is too small, need a new allocation
		if(dataSize<NElements)
		{
			if(data!=NULL)
					delete[] data;
			data= new T[NElements];
			//std::cout<<"Allocated "<<data<<" of size "<<NElements<<std::endl;
			dataSize=NElements;
		}
		//Otherwise, do nothing, we have enough room
	}

	void copy(const Matrix<T>& old){
		NRows=old.NRows;
		NCols=old.NCols;
		if(NCols==0 && NCols==0)
			return;
		allocateData(NRows*NCols);
		for (unsigned int i = 0; i < NRows*NCols; ++i) {
			assert(i<dataSize);
			data[i]=old.data[i];
		}
	}

	bool isAlphaNum(char c)
	{
		return (c>='0' && c<= '9') || (c>='a' && c<= 'Z')  || c=='.' || c=='-'|| c=='+' ;
	}
};


template<typename T> std::ostream& operator<<(std::ostream& os, const Matrix<T>& m)
{
	m.toStream(os);
	return os;
}





#endif /* MATRIX_H_ */










