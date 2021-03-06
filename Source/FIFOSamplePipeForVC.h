#pragma once
#include <cassert>
#include <iostream>
typedef double SAMPLETYPE;
typedef unsigned int uint;
#define THROW_RT_ERROR(x)    {throw std::runtime_error(x);}
class FIFOSamplePipe
{
protected:

    bool verifyNumberOfChannels(int nChannels) const
    {
        if ((nChannels > 0) && (nChannels <= 16))
        {
            return true;
        }
		THROW_RT_ERROR("Error: 轨道数目不对");
        return false;
    }

public:
    virtual ~FIFOSamplePipe() {}


    /// 返回一个指向输出样本开头的指针。
     /// 此函数用于直接访问输出样本。用完ptrBegin需要用receiveSamples报告已经弄完这些样本点了
    virtual SAMPLETYPE* ptrBegin() = 0;

    //实际的处理模块
    virtual void putSamples(const SAMPLETYPE* samples,  //指向样本的指针
        uint numSamples             ///需要处理的样本数量
    ) = 0;


    // 将其他音频流移动到这个音频流
    void moveSamples(FIFOSamplePipe& other  ///<接收其他该结构实例
    )
    {
        int oNumSamples = other.numSamples();

        putSamples(other.ptrBegin(), oNumSamples);
        other.receiveSamples(oNumSamples);
    };

    /// 从样本缓冲区的开头输出样本。 将要求样本复制到
	/// 输出缓冲区并将它们从样本缓冲区中删除。 如果少于
	/// 缓冲区中的“numsample”样本，返回所有可用的样本。

    virtual uint receiveSamples(SAMPLETYPE* output, ///<指针指向应该输出的双环形缓冲区
        uint maxSamples                 ///< 应该输出多少
    ) = 0;

    /// Adjusts book-keeping so that given number of samples are removed from beginning of the 
    /// sample buffer without copying them anywhere. 
    ///
    /// Used to reduce the number of samples in the buffer when accessing the sample buffer directly
    /// with 'ptrBegin' function.
    virtual uint receiveSamples(uint maxSamples   ///< Remove this many samples from the beginning of pipe.
    ) = 0;

    /// Returns number of samples currently available.
    virtual uint numSamples() const = 0;

    // Returns nonzero if there aren't any samples available for outputting.
    virtual int isEmpty() const = 0;

    /// Clears all the samples.
    virtual void clear() = 0;

    /// allow trimming (downwards) amount of samples in pipeline.
    /// Returns adjusted amount of samples
    virtual uint adjustAmountOfSamples(uint numSamples) = 0;

};


/// Base-class for sound processing routines working in FIFO principle. With this base 
/// class it's easy to implement sound processing stages that can be chained together,
/// so that samples that are fed into beginning of the pipe automatically go through 
/// all the processing stages.
///
/// When samples are input to this class, they're first processed and then put to 
/// the FIFO pipe that's defined as output of this class. This output pipe can be
/// either other processing stage or a FIFO sample buffer.
class FIFOProcessor :public FIFOSamplePipe
{
public:
    /// Internal pipe where processed samples are put.
    FIFOSamplePipe* output;

    /// Sets output pipe.
    void setOutPipe(FIFOSamplePipe* pOutput)
    {
        assert(output == NULL);
        assert(pOutput != NULL);
        output = pOutput;
    }

    /// Constructor. Doesn't define output pipe; it has to be set be 
    /// 'setOutPipe' function.
    FIFOProcessor()
    {
        output = NULL;
    }

    /// Constructor. Configures output pipe.
    FIFOProcessor(FIFOSamplePipe* pOutput   ///< Output pipe.
    )
    {
        output = pOutput;
    }

    /// Destructor.
    virtual ~FIFOProcessor()
    {
    }

    /// Returns a pointer to the beginning of the output samples. 
    /// This function is provided for accessing the output samples directly. 
    /// Please be careful for not to corrupt the book-keeping!
    ///
    /// When using this function to output samples, also remember to 'remove' the
    /// output samples from the buffer by calling the 
    /// 'receiveSamples(numSamples)' function
    virtual SAMPLETYPE* ptrBegin()
    {
        return output->ptrBegin();
    }

public:

    /// Output samples from beginning of the sample buffer. Copies requested samples to 
    /// output buffer and removes them from the sample buffer. If there are less than 
    /// 'numsample' samples in the buffer, returns all that available.
    ///
    /// \return Number of samples returned.
    virtual uint receiveSamples(SAMPLETYPE* outBuffer, ///< Buffer where to copy output samples.
        uint maxSamples                    ///< How many samples to receive at max.
    )
    {
        return output->receiveSamples(outBuffer, maxSamples);
    }

    /// Adjusts book-keeping so that given number of samples are removed from beginning of the 
    /// sample buffer without copying them anywhere. 
    ///
    /// Used to reduce the number of samples in the buffer when accessing the sample buffer directly
    /// with 'ptrBegin' function.
    virtual uint receiveSamples(uint maxSamples   ///< Remove this many samples from the beginning of pipe.
    )
    {
        return output->receiveSamples(maxSamples);
    }

    /// Returns number of samples currently available.
    virtual uint numSamples() const
    {
        return output->numSamples();
    }

    /// Returns nonzero if there aren't any samples available for outputting.
    virtual int isEmpty() const
    {
        return output->isEmpty();
    }

    /// allow trimming (downwards) amount of samples in pipeline.
    /// Returns adjusted amount of samples
    virtual uint adjustAmountOfSamples(uint numSamples)
    {
        return output->adjustAmountOfSamples(numSamples);
    }
};