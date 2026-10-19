#!/usr/bin/env python3
import argparse
import subprocess
import os

bin_home = os.environ['GLM_HOME']

def main():
    args = parse_args()
    num_proc_threads = str(args.num_query_processor_threads)
    num_node_lock_handlers = str(args.num_remote_node_lock_handlers)
    logs_dir = args.logs_dir 

    glm_tps_per_run = [None] * args.num_runs
    glm_binary_and_args = [
        bin_home + '/build/release/apps/compute_node',
        str(args.glmNodeId), # 1. nodeId hosting the processor;
        args.logs_dir, # 2. absolute path to the logs dir ending in '/';
        str(args.len_experiments_in_secs), # 3. length of experiment in secs;
        num_proc_threads, # 4. numb of processor threads; and
        num_node_lock_handlers # 5. num of handling node lock request handlers.
    ]

    non_glm_tps_per_run = [None] * args.num_runs
    non_glm_binary_and_args = [
        bin_home + '/build/release/apps/compute_node',
        '2', # 1. nodeId hosting the processor;
        args.logs_dir, # 2. absolute path to the logs dir ending in '/';
        str(args.len_experiments_in_secs), # 3. length of experiment in secs;
        num_proc_threads, # 4. num of processor threads; and
        num_node_lock_handlers, # 5. num of handling node lock request handlers.
    ]

    print(glm_binary_and_args)
    if args.glmNodeId > 0:
        print(non_glm_binary_and_args)
 
    for i in range(args.num_runs):
        if args.logs_dir != 'NULL':
            glm_binary_and_args[2] = args.logs_dir + '/' + str(i + 1) + '/'
            non_glm_binary_and_args[2] = args.logs_dir + '/' + str(i + 1) + '/'
        print('starting run ' + str(i + 1) + '/' + str(args.num_runs))

        glm_log_filename = bin_home + '/glm_stdout_' + num_proc_threads + \
            '_' + num_node_lock_handlers + '.log'
        non_glm_log_filename = bin_home + '/non_glm_stdout_' + num_proc_threads + \
            '_' + num_node_lock_handlers + '.log'

        glm_stdout_log = open(glm_log_filename, 'w')
        glm_popen = subprocess.Popen(tuple(glm_binary_and_args), \
            stdout=glm_stdout_log, stderr=glm_stdout_log)

        if args.glmNodeId > 0:
            non_glm_stdout_log = open(non_glm_log_filename, 'w')
            non_glm_popen = subprocess.Popen(tuple(non_glm_binary_and_args), \
                stdout=non_glm_stdout_log, stderr=non_glm_stdout_log)
            non_glm_popen.wait()
            non_glm_stdout_log.close()

        glm_popen.wait()
        glm_stdout_log.close()

        print('collecting logs for run ' + str(i + 1) + '/' + str(args.num_runs))

        # splits logs into multiples ones.

        with open(glm_log_filename, 'r') as stdout_logs:
            for line in stdout_logs:
                if "#TPS/core : " in line:
                    glm_tps_per_run[i] = int(float(line.split("#TPS/core : ")[1].split('\n')[0]))

        if args.glmNodeId > 0:
            with open(non_glm_log_filename, 'r') as stdout_logs:
                for line in stdout_logs:
                    if "#TPS/core : " in line:
                        non_glm_tps_per_run[i] = int(float(line.split("#TPS/core : ")[1].split('\n')[0]))

    print(glm_tps_per_run)
    if args.glmNodeId > 0:
        print(non_glm_tps_per_run)

def parse_args():
    parser = argparse.ArgumentParser(
        description='run transactions on glm and non-glm nodes.')
    parser.add_argument('glmNodeId', help='node id for the glm',
        type=int, default=0)
    parser.add_argument('len_experiments_in_secs', help='time in seconds \
        for which the experiments runs', type=int)
    parser.add_argument('num_query_processor_threads', help='number of query \
        processor threads executing transactions', type=int)
    parser.add_argument('num_remote_node_lock_handlers', help='number of \
        remote GetNodeLock request handler threads', type=int)
    parser.add_argument('-d', '--logs_dir', help='absolute log directory ending in /',
        default='NULL')
    parser.add_argument('-r', '--num_runs', help='number of runs', type=int, default=3)
    parser.add_argument('-t', '--total_num_pages', help='total # pages in \
        the table', type=int, default=1000)
    parser.add_argument('-s', '--num_shared_pages', help= '# shared pages out of total',
        type=int, default=100)
    return parser.parse_args()

if __name__ == '__main__':
    main()

