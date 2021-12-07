---
documentclass: article
title: An Overview of WLAN
author: Wu Zhenyu (SA21006096)
institute: USTC
bibliography: refs/main.bib
---

## Background

### History

Wireless Local Area Networks, or WLANs for short, appear in the market in the
forth-coming years based on the amendments to the IEEE 802.11 standards that
have recently been approved or are under development.

- IEEE 802.11aa (Robust Audio Video Transport Streaming)
- IEEE 802.11ac (Very-high throughput at < 6GHz)
- IEEE 802.11af (TV White Spaces)
- IEEE 802.11ah (Machine-to-Machine communications)

IEEE 802.11 is a standard for Wireless Local Area Networks, in another
well-known name as WiFi. From the birth of year 1997, the technology has 25
years of history, which represents it have become a very mature technology.
In the beginning, WLAN is just an extension of wired LANs using Ethernet
technology. With the development of IEEE 802.11 standard, WLAN are widely
available everywhere (More than two billion WLAN-enabled devices [@5394032]).
To analysis the reason, there are as following:

- interoperablity
- ease of use
- flexibility

### Features

#### Frequency

The 2.4 GHz and 5 GHz frequency bands, the spectrum IEEE 802.11 standards used,
is unlicensed. Anyone can deploy a WLAN in those bands, which brings a
challenge to guarantee performance bounds and reasonable Quality of Service
(QoS) levels. Network Densification exacerbated the problem.[@6736747] Emerging
trend of deploying a large number of base stations in hotspot areas to cope
with increase in traffic demands.

#### CSMA/CA

Carrier Sense Multiple Access with Collision Avoidance (CSMA/CA) is a media
access control (MAC) protocol. Because of half duplex(i.e., A sending station
cannot receive a carrier-sense), collision is impossible to be detected like
CSMA/CD which Ethernet adopted. CSMA/CA is simple and cheap to implement.
Moreover, CSMA/CA is scalable, which means it can provide easy support for
mobility and decentralized network architectures, whatever classical ad hoc
networks or emerging people-centric networks. On the worst situation, CSMA/CA
can only pay a best effort on transmission service. In order to support better
QoS, major efforts have been dedicated in the amendments of IEEE 802.11e.

### Technology

- Orthogonal frequency-division multiplexing (OFDM) allowed achieving maximum
data rates up to 54 Mb/s.
- Multiple-input multiple-output (MIMO) made performance of WLANs close to that
of a wired Ethernet network. [@654749]
- vehicle-to-vehicle (V2V) and vehicle-to-infrastructure (V2I) communication
(together referred to as V2X) in the 5.9 GHz band, which is licensed for
Intelligent Transportation Systems (ITS).
- Wireless mesh networks should operate on top of the existing IEEE 802.11
MAC protocol

Now, IEEE 802.11 working group was also rapidly moving its focus towards
next-generation WLANs. They forecasted three key drivers:

- Machine-to-Machine communications
- High Definition Multimedia Communications
- "Spectrum Sharing" in licensed bands by using cognitive radio

Specifically, the emergence of the Internet of Things (IoT) vision requires a
low-power WLAN technology [@6211498]. At the same time, the widespread diffusion
of mobile devices with diverse networking and multimedia capabilities, as well
as the wide adoption of advanced multimedia applications, is fueling the
growth of mobile video traffic, which was already more than half of the global
mobile data traffic by the end of 2013.

## Scenarios

Nowadays, WLANs provide a fast and reliable wireless access to Internet for
browsing the web, exchanging files, chatting, receiving and answering e-mails,
and for low-quality realtime audio/video streams. This clearly results in a
steady increase of the Internet traffic. Two representative examples of the
change in Internet use are:

- the high demand for mobile-rich multi-media content, mainly motivated by the
use of smart-phones, tablets and other multimedia portable devices;
- the increasing interest in IoT applications driven by the almost ubiquitous
existence of devices able to collect data from the environment, ranging from
low-power sensor nodes to connected cars.

Therefore, WLANs must also evolve to provide effective solutions to these new
upcoming scenarios, and the challenges they pose to satisfy their requirements.
Four of the key use cases for next-generation WLANs are discussed in the
following subsections.

Three key scenarios in which next-generation WLANs will have to operate.

- Machine-to-Machine (M2M) Communications
- High-Quality Multimedia Content Delivery
- Efficient Use of the Spectrum

## Multimedia Applications

These three amendments target multimedia scenarios by introducing new
physical-layer technologies and MAC functionalities to improve the WLAN
capacity and QoS provision. Application examples include home scenarios in
which an WLAN AP can act as an Internet gateway and wireless media server for
home appliances (e.g., IPTV set-top boxes, projectors, game consoles) and
content storage devices.

Since the IEEE 802.11ac amendment has recently been finalized, current research
around it should cover two main aspects:

- understanding the performance bounds of IEEE 802.11ac, which entails the
development of new models, simulation tools and experimental platforms of IEEE
802.11ac-based WLANs.
- proposing specific solutions for those aspects that are not defined by the
IEEE 802.11ac amendment on purpose, such as the mechanism for creating the
groups of STAs for DL-MU-MIMO transmissions, smart packet schedulers able to
decide when the use of DL-MU-MIMO outperforms SU-MIMO transmissions, and the
implementation of the TXOP sharing feature between several ACs. The results and
conclusions obtained in both cases will be very valuable in the development of
IEEE 802.11ac technologies, as well as in the conception of the future
amendments that will substitute IEEE 802.11ac in four to five years, such as
the recently initiated IEEE 802.11ax.

In 2014 the High Efficiency WLANs (HEW) Task Group initiated the development of
a new IEEE 802.11 amendment, called IEEE 802.11ax. The IEEE 802.11ax amendment
is expected to be released in 2019, and, to some extent, it will be the IEEE
802.11 response to the expected challenges of future dense WLAN and high
bandwidth demanding scenarios.

### Challenges

The open challenges that are considered in the development of the IEEE 802.11ax
amendment are to:

1. Improve the WLANs performance by providing at least a four-fold capacity
   increase compared to IEEE 802.11ac.
2. Provide support for dense networks, considering both the existence of
   multiple overlapping WLANs and many STAs in each of them. Spatial reuse of
   the transmission resources is a must.
3. Achieve an efficient use of the transmission resources by minimizing the
   exchange of management and control packets, revisiting the structure of the
   packets, and improving channel access and retransmission mechanisms, among
   others aspects.
4. Provide backward compatibility with previous amendments. This is achieved by
   the mandatory transmission of the legacy PHY preamble in all frames, and by
   keeping EDCA as the basic channel access scheme.
5. Introduce effective energy saving mechanisms to minimize the energy
   consumption.
6. Support multi-user transmission strategies by further developing MU-MIMO and
   Orthogonal Frequency Division Multiple Access (OFDMA) capabilities in both
   downlink and uplink.

The additional functionalities and the new technologies they include:

- multi-user MIMO techniques
- groupcast communications
- dynamic channel bonding
- spectrum databases
- channel sensing
- enhanced power saving mechanisms
- efficient small data transmissions

### Focus

The IEEE 801.11ax Task Group is currently working in four areas: PHY, MAC,
Multi-user, and Spatial Reuse. Next, we will overview some of the topics
currently under discussion in the IEEE 802.11 Task Group in each one.

**PHY layer**. The IEEE 802.11ax PHY layer will be an evolution of the IEEE 802.11ac
one. The challenges in the design of the IEEE 802.11ax PHY layer are related
with the extensions required to support multi-user MU-MIMO and OFDMA
transmissions, and Dynamic CCA. Also, improvements in the supported modulation
and channel coding techniques will be likely considered to allow for higher
transmission rates at lower SNR values. For example, IEEE 802.11ax may consider
LDPC (Low-Density Parity Check) coding, which are optional in IEEE 802.11ac,
instead of the traditional convolutional codes, as they provide a coding gain
of 1-2 dB. Moreover, the PHY layer may also include some functionalities
to support the use of Hybrid ARQ schemes to improve the efficiency of packet
retransmissions.

**Medium Access Control**. In order to keep backward compatibility
with previous IEEE 802.11 amendments, besides a common PHY frame preamble,
compatible MAC protocols are required. This means that it is likely that EDCA
will be kept as the main channel access technique in the IEEE 802.11ax
amendment. Therefore, the most relevant open challenges are related to EDCA
extensions to support a large number of STAs, improve traffic differentiation
capabilities, improve the energy consumption and provide mechanisms to fairly
co-exist with neighboring wireless networks.

**Multi-User**. Multi-user communications will likely be one of the main
characteristics of IEEE 802.11ax, as both uplink and downlink MU-MUMO and OFDMA
are under consideration. The use of multi-user communication techniques does
not necessarily represent a system capacity increase because the available
transmission resources may be the same as in the single-user communication
case. However, in WLANs, the simultaneous transmission from different users is
able to parallelize the large temporal overheads of each transmission (i.e.,
DIFS, SIFS, ACKs, packet headers, etc.) which can notably improve the WLAN
efficiency.

**Spatial Reuse**. Dense WLAN deployments are necessary
to offer a continuous coverage with high transmission rates.
To improve both the co-existence with those neighboring
networks and the spatial reuse of the spectrum, a WLAN
has two options:

- Minimize its area of influence by reducing its transmit power
- Accept higher interference levels by increasing the Clear Channel Assessment
(CCA) level.

Use of both techniques may increase the number of
concurrent transmissions between neighboring WLANs,
and therefore their capacity, although it may also result in
the opposite effect since the achievable transmission rates
may be negatively affected by the higher interference levels
observed, which is the main challenge to be solved.

In most audio/video streaming applications a group of clients must receive the
same stream simultaneously. A multicast protocol is necessary to avoid that the
same content is replicated throughout the network. In wireless networks,
multicast transmission can exploit the intrinsic broadcast nature of the
wireless channel, i.e., broadcast transmissions from an AP are physically
received by all other stations in the same collision domain. However, multicast
and broadcast frames in IEEE 802.11 networks are not protected by an
acknowledgment mechanism as in the case of unicast frames. Thus, layer-2
multicast transmissions defined by legacy IEEE 802.11 standards are unreliable
and not suitable for streaming applications. To partially address this
limitation, the Direct Multicast Service (DMS) was first specified in the IEEE
802.11v amendment [@6364431]. Basically, DMS converts multicast streams into unicast
streams. In this way, frames destined to a multicast address are individually
transmitted as unicast frames to the stations that joined that multicast group.
Obviously DMS provides the same reliability as unicast transmission services
but the consumed bandwidth increases linearly with the number of group members.
To address this scalability issue, IEEE 802.11aa includes the Groupcast with
Retries (GCR) service in addition to DMS. Specifically, the GCR service defines
new mechanisms and the related management frames for group formation, which
allows a set of stations to agree on a shared (non-multicast) address, called
the groupcast concealment address.

## Communications

M2M communications refer to any communication technology that enables
sensor/actuator devices to exchange information and perform actions without the
manual assistance of humans. This section reviews the main features currently
under consideration in the development of the upcoming IEEE 802.11ah amendment,
which targets the main challenges of those networks, such as the energy
consumption or the management of many devices.

## Radio Technology

IEEE 802.11ac adopt concepts such as the OFDM, multiuser beam-forming,
contiguous and non contiguous channel bonding and packet aggregation. Among the
mandatory and innovative behavioral and operational parameters, the most
notable one is the channel acquisition support realized through remote
geolocation-based spectrum allocation databases, which maintain the channels'
availability information in any given area and time of day, providing upon
request the list of free channels available for use.

IEEE 802.11af provided in related works and standardization initiatives to
resolve classical problems, properly characterized in the new framework of TVWS
technologies.

Channel acquisition: spectrum database and channel sensing. Much of the
attention in operating in the TVWS is given to the protection of the primary
licensed users in the TVWS spectrum band. In general, the primary users were
considered as the Digital TV (DTV) broadcasters and receivers.

Support for co-existence mechanisms so that multiple technologies can
effectively utilize the TVWS spectrum is important. Self co-existence between
network devices of a common technology (e.g. deployed by different operators in
the same area), and co-existence among different technologies, are relevant
topic of research for Cognitive Radio (CR) systems, and specifically for IEEE
802.11af. Many solutions appeared on the research scene, but no one was so far
finalized as the target solution for IEEE 802.11af. Specific standardization
has been started to regulate coexistence between wireless standards of
unlicensed devices, including the IEEE 802.19.1. The purpose of the IEEE
802.19.1 standard is to enable the family of IEEE 802 Wireless Standards to
most effectively use TV White Space by providing standard coexistence methods
among dissimilar or independently operated TVWS devices. Early examples of
generalized coexistence mechanisms included Dynamic Frequency Selection (DFS),
Transmission Power Control (TPC), listen before talk (e.g., for contention
based IEEE 802.11, 802.15), time division multiplexing (also among different
techniques such as the IEEE 802.16, 802.20, 802.22), and Message-based Spectrum
Contention (that is, beaconing messages that carry coexistence information).
Opportune metrics must be defined to assess the measurable coexistence achieved
among different technologies: as an example, the hidden node probability for a
target scenario, or the estimate of percentage variation in normalized network
throughput and latency (before and during the SU transmissions). On the other
hand, a centralized coexistence control mechanism could be effectively realized
by a central manager (or coexistence-DB, like the GDB) in critical scenarios.
To this end, IEEE 1900.4 (a standard for heterogeneous networks in dynamic
spectrum context, part of IEEE Standards Coordinating Committee 41) aims to
standardize the overall system architecture and information exchange between
the network and mobile devices, which will allow these elements to optimally
choose from available radio resources.

A relevant novel feature of IEEE 802.11af is the potential for contiguous and
non contiguous channel bonding, which permits aggregating basic channels (also
non adjacent ones), and leveraging the possible large frequency spread between
multiple available channels. Due to the rather static nature of primary DTV
transmissions, where a busy channel is unlikely to be come free in the near
future, and state changes are coarse grained in general, it is crucial to
exploit the time-locality effect and exploit the maximum physical channel
availability that could be aggregated at any given location. With the
methodology inherited from IEEE 802.11ac, IEEE 802.11af is capable of bonding
together two up to four basic channels grouped in up to two different
non-contiguous chunks. The spectrum bandwidth of a DVB-T basic channel can be
either 6, 7, or 8 MHz, depending on the country in which the service is
operated. As an example, this creates a 144 to 168 OFDM channels’ bandwidth
potential when up to four 6-7-8 MHz channels are bonded.

## New Trends

### Programmable Wireless LANs

Especially in the enterprise environment, WLAN deployments need to support a
wide range of functionalities and services. This is intrinsically difficult
because of the large number of APs that must be managed, which calls for
scalable solutions. Typical services include channel assignment, load balancing
among APs, authentication, authorization and accounting (AAA), policy
management, support for client mobility and interference coordination. Another
problem is the fact that WLAN clients autonomously take several decisions such
as which APs to associate with, when to hand-over, etc. Therefore, supporting
roaming clients requires the management of a large number of association states
across several APs, which is a challenge if support for real-time hand-over is
desired. Typically, such management schemes are centralised and most of them
are proprietary, such as WLAN controller solutions from Aruba [@ARUBA] or Cisco
[@CISCO], although the 802.11u amendment has been released to allow mobile
users to seamless roam between WiFi networks with automatic authentication and
handoff [@Cui2014PolicybasedFC]. For example, Dyson [@10.5555/1855840.1855855]
enables STAs to send information such as radio channel conditions to a
centralized controller based on a custom API (e.g., based on Python). As the
controller has a centralized view of the network, it can enforce a rich set of
policies to control the network also using historical information. A demo
system has been implemented along with applications such as airtime
reservations for specific clients or optimized handoffs. However, Dyson
requires STAs to be modified in order to use those new services offered by the
centralized controller. TRANTOR [@10.5555/1387589.1387595] is another example
of a centralised management system.

### Prototyping and Testing IEEE 802.11 Enhancements

Most of the new proposals for next-generation WLANs are currently only
evaluated using mathematical analysis and simulation. While both analysis and
simulation are necessary to characterize and study those enhancements in the
initial design phase or to consider large-scale scenarios, it is difficult to
consider all practical aspects of a real-world scenario. This can sometimes
cause significant differences between what simulations and real experiments
show. However, real experiments are challenging because of the high complexity
and costs of building the new hardware and software for each specific solution
to test.

### Cellular/WLAN Interworking

Public hotspots that offer Internet access over a WLAN using IEEE 802.11
technology are now nearly ubiquitous. It is forecasted that the cumulative
installed base of WiFi hotspots worldwide will amount to 55.1 millions by 2018,
excluding private hotspots (e.g., WiFi access points deployed at home) [@WBA].
The sharp increase in the availability of public WiFi was initially perceived
by mobile cellular operators as a threat due to the additional competition from
wireline Internet service providers or emerging crowd sourced WiFi networks,
such as FON5. However, as cellular operators are fighting to cope with the
explosion of mobile data traffic created by the rising use of multimedia
content traffic over mobile devices [@cis], they are also starting to use WLANs
based on the IEEE 802.11 technology to offload data from their core and access
networks. In general, mobile data offloading refers to the use of complementary
network technologies (in licensed or unlicensed spectrum) for
delivering data originally targeted to cellular networks. Intuitively, the
simplest type of offloading consists of exploiting connectivity to existing
co-located WiFi networks and transferring data without any delay. Thus, this
offloading technique is know as on-the-spot offloading. As a consequence of
this new trend, the seamless integration of cellular (e.g., 3G/LTE) and WiFi
technologies has attracted significant research interest in recent years (see
[@6953022] for a survey), a few solutions have already been standardized
[@6007077], and roaming between cellular and WiFi is becoming increasingly
transparent to end users. Cellular/WLAN interworking is also fostered by the
support in the evolving 4G standards of heterogeneous network deployments
(HetNets), in which the existing macro cells are complemented with a number of
small, low-power base stations with the goal of increasing capacity in highly
congested areas [@6476878]. It is envisaged that small cells will be based on
4G standards (e.g., pico and femto cells) as well as IEEE 802.11 technologies,
and multimode base stations that work simultaneously with LTE and WiFi are
already entering the market.
