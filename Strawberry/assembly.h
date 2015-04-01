/*
 * assembly.h
 *
 *  Created on: Mar 18, 2015
 *      Author: ruolin
 */

#ifndef ASSEMBLY_H_
#define ASSEMBLY_H_
#include<vector>
#include <lemon/list_graph.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include<boost/version.hpp>
#if (BOOST_VERSION < 103800)
#include <boost/vector_property_map.hpp>
#else
#include<boost/property_map/vector_property_map.hpp>
#endif

//namespace lemon{
//   class ListDigraph;
//}
class IntronTable;
class GenomicFeature;
class Contig;
typedef lemon::ListDigraph Graph;
class FlowNetwork{
   Graph _g;
   Graph::Node _source;
   Graph::Node _sink;
   static bool hasFlow(const Graph &g, const Graph::ArcMap<int> & flow, const Graph::Node node){
      for(Graph::OutArcIt out(g, node); out != lemon::INVALID; ++out){
         if (flow[out] > 0)
            return true;
      }
      return false;
   }

   static void add_sink_source(Graph &g, Graph::Node &source, Graph::Node &sink);

   static bool comp_lt(const std::pair<uint, bool> & lhs, const std::pair<uint,bool> &rhs);

   static bool search_left(const GenomicFeature &lhs, const uint rhs);

   static bool search_right(const GenomicFeature & lhs, const uint rhs);

   static void flowDecompose(const Graph &g,
         const Graph::ArcMap<int> &flow,
         const Graph::Node &source,
         const Graph::Node &sink,
         std::vector<std::vector<Graph::Arc>> &paths );

   static void flow2Transcript(const Graph &g,
         const std::vector<std::vector<Graph::Arc>> &paths,
         std::vector<std::vector<GenomicFeature>> &transcripts){

   }
public:
   void initGraph(const int &left,
           const std::vector<float> &exon_doc,
           const std::vector<IntronTable> &intron_counter,
           const std::vector<size_t> &bad_introns,
           std::vector<GenomicFeature> &exons);

   void splicingGraph(const int &left, const std::vector<float> &exon_doc,
         const std::vector<IntronTable> &intron_counter, const std::vector<size_t> &bad_introns,
         std::vector<GenomicFeature> &exons);

   void createNetwork(const std::vector<GenomicFeature> &exons,
         const std::vector<IntronTable> &intron_counter,
         const std::vector<size_t> &bad_introns);

   void addWeight(const std::vector<Contig> &hits, const std::vector<IntronTable> &intron_counter);
   void solveNetwork();
};

#endif /* ASSEMBLY_H_ */
