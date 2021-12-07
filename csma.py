#! /usr/bin/env python3
"""CSMA/CD Algorithm.

usage: main.py [-hVn] [-t <time>] [-o <dir>]

options:
    -h, --help                  Show this screen.
    -V, --version               Show version.
    -t, --time <time>           Simulation time, unit: s. [default: 3600]
    -o, --output <dir>          Save output to a directory.
    -n, --dry-run               Don't open any window.
"""
import random
import math
import collections
import os
from matplotlib import pyplot as plt
from mpl_toolkits.axes_grid1 import host_subplot


class Node:
    """Node."""

    def __init__(self, location, A, maxSimulationTime=3600):
        """__init__.

        :param location:
        :param A:
        :param maxSimulationTime:
        """
        self.location = location  # Defined as a multiple of D
        self.collisions = 0
        self.wait_collisions = 0
        self.MAX_COLLISIONS = 10
        self.maxSimulationTime = maxSimulationTime
        if A:
            self.queue = collections.deque(self.generate_queue(A))
        else:
            self.queue = [float("infinity")]

    def exponential_backoff_time(self, R, general_collisions):
        """exponential_backoff_time.

        :param R:
        :param general_collisions:
        """
        rand_num = random.random() * (pow(2, general_collisions) - 1)
        return rand_num * 4096 / float(R)  # 4096 bit-times

    def pop_packet(self):
        """pop_packet."""
        self.queue.popleft()
        self.collisions = 0
        self.wait_collisions = 0

    def collision_occured(self, R):
        """collision_occured.

        :param R:
        """
        self.collisions += 1
        if self.collisions > self.MAX_COLLISIONS:
            # Drop packet and reset collisions
            return self.pop_packet()

        # Add the exponential backoff time to waiting time
        backoff_time = self.queue[0] + self.exponential_backoff_time(
            R, self.collisions
        )

        for i in range(len(self.queue)):
            if backoff_time >= self.queue[i]:
                self.queue[i] = backoff_time
            else:
                break

    def successful_transmission(self):
        """successful_transmission."""
        self.collisions = 0
        self.wait_collisions = 0

    def generate_queue(self, A):
        """generate_queue.

        :param A:
        """
        packets = []
        arrival_time_sum = 0

        while arrival_time_sum <= self.maxSimulationTime:
            arrival_time_sum += get_exponential_random_variable(A)
            packets.append(arrival_time_sum)
        return packets

    def non_persistent_bus_busy(self, R):
        """non_persistent_bus_busy.

        :param R:
        """
        self.wait_collisions += 1
        if self.wait_collisions > self.MAX_COLLISIONS:
            # Drop packet and reset collisions
            return self.pop_packet()

        # Add the exponential backoff time to waiting time
        backoff_time = self.queue[0] + self.exponential_backoff_time(
            R, self.wait_collisions
        )

        for i in range(len(self.queue)):
            if backoff_time >= self.queue[i]:
                self.queue[i] = backoff_time
            else:
                break


def get_exponential_random_variable(param):
    """get_exponential_random_variable.

    :param param:
    """
    return -math.log(1 - random.uniform(0, 1)) / float(param)


def csma_cd(nodes, R, L, S, is_persistent):
    """csma_cd.

    :param nodes:
    :param R: The speed of the LAN/channel/bus (in bps)
    :param L: Packet length (in bits)
    :param S: Propagation speed (meters/sec)
    :param is_persistent:
    """
    curr_time = 0
    transmitted_packets = 0
    successfuly_transmitted_packets = 0

    while True:

        # Step 1: Pick the smallest time out of all the nodes
        min_node = Node(None, None)  # Some random temporary node
        for node in nodes:
            if len(node.queue) > 0:
                min_node = (
                    min_node if min_node.queue[0] < node.queue[0] else node
                )

        # Terminate if no more packets to be delivered
        if min_node.location is None:
            break

        curr_time = min_node.queue[0]
        transmitted_packets += 1

        # Step 2: Check if collision will happen
        # Check if all other nodes except the min node will collide
        collsion_occurred_once = False
        for node in nodes:
            if node.location != min_node.location and len(node.queue) > 0:
                delta_location = abs(min_node.location - node.location)
                t_prop = delta_location / float(S)
                t_trans = L / float(R)

                # Sense bus busy
                if (
                    curr_time + t_prop
                    < node.queue[0]
                    < curr_time + t_prop + t_trans
                ):
                    if is_persistent:
                        for i in range(len(node.queue)):
                            if (
                                curr_time + t_prop
                                < node.queue[i]
                                < curr_time + t_prop + t_trans
                            ):
                                node.queue[i] = curr_time + t_prop + t_trans
                            else:
                                break
                    else:
                        node.non_persistent_bus_busy(R)

                # Check collision
                if node.queue[0] <= curr_time + t_prop:
                    collsion_occurred_once = True
                    transmitted_packets += 1
                    node.collision_occured(R)

        # Step 3: If a collision occured then retry
        # otherwise update all nodes latest packet arrival times and proceed to the next packet
        if collsion_occurred_once is not True:  # If no collision happened
            successfuly_transmitted_packets += 1
            min_node.pop_packet()
        else:  # If a collision occurred
            min_node.collision_occured(R)

    efficiency = successfuly_transmitted_packets / transmitted_packets
    return efficiency, successfuly_transmitted_packets


def draw(Ns, efficiencies, throughputs, name="nodes", savedir=None):
    fig = plt.figure(name)
    ax = host_subplot(111, figure=fig)
    ax2 = ax.twinx()
    ax.set_xlabel(name)
    ax.set_ylabel("efficiency")
    ax2.set_ylabel("throughput")
    ax.plot(Ns, efficiencies, label="efficiencies")
    ax2.plot(Ns, throughputs, label="throughput")
    plt.legend()
    if savedir:
        fig.savefig(os.path.join(savedir, name + ".png"))

C = 3 * pow(10, 8)  # speed of light
S = (2 / float(3)) * C

# Show the system efficiency, block delay, and block throughput (CSMA/CD Persistent)
if __name__ == "__main__" and __doc__:
    from docopt import docopt
    from typing import Dict, Union, List

    Arg = Union[bool, int, str, List[str]]
    args: Dict[str, Arg] = docopt(
        __doc__, version="v0.0.1", options_first=True
    )

    maxSimulationTime = int(args["--time"])  # type: ignore
    R = 1 * pow(10, 9)
    L = 1500

    throughputs = []
    efficiencies = []
    D = 1500_000
    A = 1 / 60
    Ns = range(10, 100, 10)
    for N in Ns:
        nodes = [Node(i * D, A, maxSimulationTime) for i in range(0, N)]
        efficiency, throughput = csma_cd(nodes, R, L, S, True)
        efficiencies.append(efficiency)
        throughputs.append(throughput)
    draw(Ns, efficiencies, throughputs, name="nodes", savedir=args["--output"])

    throughputs = []
    efficiencies = []
    D = 1500_000
    N = 100
    fs = range(60, 360, 60)
    for f in fs:
        A = 1/f
        nodes = [Node(i * D, A, maxSimulationTime) for i in range(0, N)]
        efficiency, throughput = csma_cd(nodes, R, L, S, True)
        efficiencies.append(efficiency)
        throughputs.append(throughput)
    draw(fs, efficiencies, throughputs, name="arrival_freqencies", savedir=args["--output"])

    throughputs = []
    efficiencies = []
    A = 1 / 60
    N = 100
    Ds = range(1500_000, 15_000_000, 1500_000)
    for D in Ds:
        nodes = [Node(i * D, A, maxSimulationTime) for i in range(0, N)]
        efficiency, throughput = csma_cd(nodes, R, L, S, True)
        efficiencies.append(efficiency)
        throughputs.append(throughput)
    draw(Ds, efficiencies, throughputs, name="distances", savedir=args["--output"])

    if not args["--dry-run"]:
        plt.show()
