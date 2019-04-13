#include <iostream>
#include <vector>
#include <cstdlib>
#include <iterator>
#include <string>
#include <algorithm>
#include <Eigen/Core>
#include <stdio.h>
#include <sys/time.h>

#include <boost/compute/core.hpp>
#include <boost/compute/event.hpp>
#include <boost/compute/system.hpp>
#include <boost/compute/algorithm/copy.hpp>
#include <boost/compute/algorithm/transform.hpp>
#include <boost/compute/async/future.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/lambda.hpp>
#include <boost/compute/program.hpp>
#include <boost/compute/kernel.hpp>
#include <boost/compute/function.hpp>
#include <boost/compute/utility/source.hpp>


namespace compute = boost::compute;


using namespace std;
using namespace Eigen;

string source = BOOST_COMPUTE_STRINGIZE_SOURCE(
__kernel void foo (__global float* a, __global float* b, __global float* c)
{
        const uint i = get_global_id(0);
        c[i] = a[i] + b[i];
        a[i] = 10;
        b[i] = 11;
        return;
}
);

string mSource = BOOST_COMPUTE_STRINGIZE_SOURCE(
__kernel void work_func (__global int* src1, const int src1Height, const int src1Width, __global int* src2, const int src2Width, __global int* dst)
{
        const uint idx = get_global_id(0);
        //int* A = src1;
//      int* B = src2 + src1Width * idx;
//      int* C = dst + src1Height * idx;
        src2 += src1Width * idx;
            dst  += src1Height * idx;

        for (int i = 0; i < src1Width; i++) {
                for (int j = 0; j < src1Height; j++) {
                        dst[j] += src1[j] * src2[i];
                }
                src1 += src1Height;
        }
        //if (!idx) {
        //   printf ("dst[idx] : ");
        //   for (int j = 0; j < src1Height; j++) {
        //      printf("%d ", dst[j]);
        //   }
        //   printf("\n");
        //}
        return;
}
);
int main()
{
        compute::device device = compute::system::default_device();
        cout << "device name: " << device.name() << endl;
        cout << "device count: " << compute::system::device_count() << endl;
        cout << "Platform count: " << compute::system::platform_count() << endl;

        compute::context context(device);

        compute::command_queue queue(
                context, device, compute::command_queue::enable_profiling);
                  /**** Trying to create a kernel ****/
//      cout << "source :(" << source << ")" << endl;
//      compute::program foo_program = compute::program::create_with_source(source, context);
//      try {
//              foo_program.build();
//      }
//      catch(compute::opencl_error &e) {
//              cout << "Build Logs: " << foo_program.build_log() << endl;
//      }
//      compute::kernel foo_kernel = foo_program.create_kernel("foo");
//
//      compute::buffer buffer_a(context, 4*sizeof(float));
//      compute::buffer buffer_b(context, 4*sizeof(float));
//      compute::buffer buffer_c(context, 4*sizeof(float));
//
//      float* a = (float *)queue.enqueue_map_buffer(buffer_a, CL_MAP_WRITE, 0, 4 * sizeof(float));
//      float* b = (float *)queue.enqueue_map_buffer(buffer_b, CL_MAP_WRITE, 0, 4 * sizeof(float));
//
//
//      generate(a, a + 4, []() {return rand() % 10 + 1;});
//      generate(b, b + 4, []() {return rand() % 10 + 1;});
//      cout << "Before a : [ " ; copy(a, a + 4, ostream_iterator<float>(cout, " ")); cout << " ]" << endl;
//      cout << "Before b : [ " ; copy(b, b + 4, ostream_iterator<float>(cout, " ")); cout << " ]" << endl;
//
//      queue.enqueue_unmap_buffer(buffer_a, a);
//      queue.enqueue_unmap_buffer(buffer_b, b);
//
//
//      foo_kernel.set_arg(0, buffer_a);
//      foo_kernel.set_arg(1, buffer_b);
//      foo_kernel.set_arg(2, buffer_c);
//      queue.enqueue_1d_range_kernel(foo_kernel, 0, 4, 0);
//
//      float* c = (float *)queue.enqueue_map_buffer(buffer_c, CL_MAP_READ, 0, 4 * sizeof(float));
//      cout << "Result is : "; copy (c, c + 4, ostream_iterator<float>(cout, " "));
//      cout << endl;
//
//      a = (float *)queue.enqueue_map_buffer(buffer_a, CL_MAP_WRITE, 0, 4 * sizeof(float));
//      b = (float *)queue.enqueue_map_buffer(buffer_b, CL_MAP_WRITE, 0, 4 * sizeof(float));
//      cout << "After a : [ " ; copy(a, a + 4, ostream_iterator<float>(cout, " ")); cout << " ]" << endl;
//      cout << "After b : [ " ; copy(b, b + 4, ostream_iterator<float>(cout, " ")); cout << " ]" << endl;
//
//      queue.enqueue_unmap_buffer(buffer_b, b);
//      queue.enqueue_unmap_buffer(buffer_a, a);
//      queue.enqueue_unmap_buffer(buffer_c, c);
//
        /** Create a kernel end **/ 
           /**** Create Matrix kernel ****/
        compute::program mProgram = compute::program::create_with_source(mSource, context);
        try {
                mProgram.build();
        }
        catch (compute::opencl_error &e) {
                cout << "Build Logs: " << mProgram.build_log() << endl;
                return 0;
        }
        compute::kernel m_kernel = mProgram.create_kernel("work_func");

for (int cycle = 0; cycle < 20; cycle++) {
        int src1Height, src1Width, src2Width;
        src1Height = rand() % 2000 + 4;
        src1Width = rand() % 2000 + 4;
        src2Width = rand() % 2000 + 4;
        int src1Len = src1Height * src1Width;
        int src2Len = src1Width * src2Width;
        int dstLen = src1Height * src2Width;
        cout << "src1Height = " << src1Height << " src1Widht = " << src1Width << " src2Width = " << src2Width << endl;

        compute::buffer src1(context, src1Len * sizeof(int));
        compute::buffer src2(context, src2Len * sizeof(int));
        compute::buffer dst(context, dstLen * sizeof(int));

        int* host_src1 = (int *)queue.enqueue_map_buffer(src1, CL_MAP_WRITE, 0, src1Len * sizeof(int));
        int* host_src2 = (int *)queue.enqueue_map_buffer(src2, CL_MAP_WRITE, 0, src2Len * sizeof(int));
        //int* host_dst = (int *)queue.enqueue_map_buffer(dst, CL_MAP_WRITE, 0, dstLen * sizeof(int));

        generate(host_src1, host_src1 + src1Len, [](){return rand() % 10 + 1;});
        generate(host_src2, host_src2 + src2Len, [](){return rand() % 10 + 1;});
//      generate(host_dst, host_src2 + src2Len, [](){return 1;});
        //cout << "src1 : [ " ; copy(host_src1, host_src1 + src1Len, ostream_iterator<int>(cout, " ")); cout << " ]" << endl;
        //cout << "src2 : [ " ; copy(host_src2, host_src2 + src1Len, ostream_iterator<int>(cout, " ")); cout << " ]" << endl;
        queue.enqueue_unmap_buffer(src1, host_src1);
        queue.enqueue_unmap_buffer(src2, host_src2);
//      queue.enqueue_unmap_buffer(dst, host_dst);
//      foo_kernel.set_arg(0, buffer_a);
//      foo_kernel.set_arg(1, buffer_b);
//      foo_kernel.set_arg(2, buffer_c);
        m_kernel.set_args(src1, src1Height, src1Width, src2, src2Width, dst);
        compute::event event = queue.enqueue_1d_range_kernel(m_kernel, 0, src2Width, 0);
        event.wait();
        boost::chrono::milliseconds duration =
                event.duration<boost::chrono::milliseconds>(CL_PROFILING_COMMAND_START, CL_PROFILING_COMMAND_END);
        cout << "OpenCL takes : " << duration.count() << " ms" << endl;
        boost::chrono::milliseconds duration1 =
                event.duration<boost::chrono::milliseconds>(CL_PROFILING_COMMAND_SUBMIT, CL_PROFILING_COMMAND_END);
        cout << "Submit takes : " << duration1.count() << " ms" << endl;


        host_src1 = (int *)queue.enqueue_map_buffer(src1, CL_MAP_READ, 0, src1Len * sizeof(int));
        host_src2 = (int *)queue.enqueue_map_buffer(src2, CL_MAP_READ, 0, src2Len * sizeof(int));
        int* host_dst = (int *)queue.enqueue_map_buffer(dst, CL_MAP_READ, 0, dstLen * sizeof(int));
        int* dst_1 = new int[dstLen];

        Map<MatrixXi> A(host_src1, src1Height, src1Width);
        Map<MatrixXi> B(host_src2, src1Width, src2Width);
        Map<MatrixXi> C(dst_1, src1Height, src2Width);

        struct timeval tv;
        struct timezone tz;

        gettimeofday(&tv, &tz);
        unsigned long long t1 = tv.tv_sec * 1000000ULL + tv.tv_usec;
        C = A * B;
        gettimeofday(&tv, &tz);
        unsigned long long t2 = tv.tv_sec * 1000000ULL + tv.tv_usec;
        cout << " Eigen takes " << (double)(t2 - t1) / 1000 << " ms." << endl;

        //cout << "src1 : [ " ; copy(host_src1, host_src1 + src1Len, ostream_iterator<int>(cout, " ")); cout << " ]" << endl;
        //cout << "src2 : [ " ; copy(host_src2, host_src2 + src1Len, ostream_iterator<int>(cout, " ")); cout << " ]" << endl;

        //cout << "host_dst [ " ; copy(host_dst, host_dst + dstLen, ostream_iterator<int>(cout, " "));  cout << " ]" << endl;
        //cout << "dst_1 [ " ; copy(dst_1, dst_1 + dstLen, ostream_iterator<int>(cout, " "));  cout << " ]" << endl;

        if (!equal(dst_1, dst_1 + dstLen, host_dst))
                cout << "Calculate Error." << endl;

        queue.enqueue_unmap_buffer(src1, host_src1);
        queue.enqueue_unmap_buffer(src2, host_src2);
         queue.enqueue_unmap_buffer(dst, host_dst);
        delete [] dst_1;
}



        return 0;

}
