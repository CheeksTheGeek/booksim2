// $Id$

/*
 Copyright (c) 2007-2015, Trustees of The Leland Stanford Junior University
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this 
 list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*unidirectional_torus.cpp
 *
 * Unidirectional Torus topology - connects only East and South
 * Based on the credit-based torus design from Lab3
 */

#include "booksim.hpp"
#include <vector>
#include <sstream>
#include <ctime>
#include <cassert>
#include "unidirectional_torus.hpp"
#include "random_utils.hpp"
#include "misc_utils.hpp"
#include "routefunc.hpp"

UnidirectionalTorus::UnidirectionalTorus( const Configuration &config, const string & name ) :
Network( config, name )
{
  _ComputeSize( config );
  _Alloc( );
  _BuildNet( config );
}

void UnidirectionalTorus::_ComputeSize( const Configuration &config )
{
  _k = config.GetInt( "k" );
  _n = config.GetInt( "n" );

  gK = _k; gN = _n;
  _size     = powi( _k, _n );
  // Only one connection per dimension per node (unidirectional)
  _channels = _n*_size;

  _nodes = _size;
}

void UnidirectionalTorus::RegisterRoutingFunctions() {
  // Register dimension-order routing for unidirectional torus
  gRoutingFunctionMap["dor_unidirectional_torus"] = &dim_order_unidirectional_torus;
}

void UnidirectionalTorus::_BuildNet( const Configuration &config )
{
  ostringstream router_name;

  //latency type, noc or conventional network
  bool use_noc_latency;
  use_noc_latency = (config.GetInt("use_noc_latency")==1);
  
  for ( int node = 0; node < _size; ++node ) {

    router_name << "router";
    
    if ( _k > 1 ) {
      for ( int dim_offset = _size / _k; dim_offset >= 1; dim_offset /= _k ) {
        router_name << "_" << ( node / dim_offset ) % _k;
      }
    }

    // For 2D unidirectional torus: 2 inputs, 2 outputs, 1 injection, 1 ejection
    // Total: 3 inputs, 3 outputs
    _routers[node] = Router::NewRouter( config, this, router_name.str( ), 
                                        node, _n + 1, _n + 1 );
    _timed_modules.push_back(_routers[node]);

    router_name.str("");

    for ( int dim = 0; dim < _n; ++dim ) {

      // Channel latency
      int latency = 2; // torus channel latency

      // Get the output channel numbers (to East and South)
      int output_channel = _EastChannel( node, dim );
      
      // Add the output channels (to East and South)
      _routers[node]->AddOutputChannel( _chan[output_channel], _chan_cred[output_channel] );

      // Set output channel latency
      if(use_noc_latency){
        _chan[output_channel]->SetLatency( latency );
        _chan_cred[output_channel]->SetLatency( latency );
      } else {
        _chan[output_channel]->SetLatency( 1 );
        _chan_cred[output_channel]->SetLatency( 1 );
      }
    }
    
    // Connect input channels from neighbors
    for ( int dim = 0; dim < _n; ++dim ) {
      // For dimension 0 (X), input comes from West neighbor
      // For dimension 1 (Y), input comes from North neighbor
      int neighbor_node;
      if (dim == 0) {
        neighbor_node = _WestNode( node, dim );
      } else {
        neighbor_node = _NorthNode( node, dim );
      }
      
      int input_channel = _EastChannel( neighbor_node, dim );
      _routers[node]->AddInputChannel( _chan[input_channel], _chan_cred[input_channel] );
    }
    
    // Injection and ejection channels, always 1 latency
    _routers[node]->AddInputChannel( _inject[node], _inject_cred[node] );
    _routers[node]->AddOutputChannel( _eject[node], _eject_cred[node] );
    _inject[node]->SetLatency( 1 );
    _eject[node]->SetLatency( 1 );
  }
}

int UnidirectionalTorus::_EastChannel( int node, int dim )
{
  // The base channel for a node is _n*node
  int base = _n*node;
  // The offset for dimension dim
  int off  = dim;
  return ( base + off );
}

int UnidirectionalTorus::_SouthChannel( int node, int dim )
{
  // For 2D torus, dim=0 is X, dim=1 is Y
  // Only for Y dimension (dim=1), we have South channel
  if (dim == 1) {
    // The base channel for a node is _n*node
    int base = _n*node;
    // The offset for South channel is dim
    int off  = dim;
    return ( base + off );
  }
  // For X dimension, return East channel
  return _EastChannel( node, dim );
}

int UnidirectionalTorus::_EastNode( int node, int dim )
{
  int k_to_dim = powi( _k, dim );
  int loc_in_dim = ( node / k_to_dim ) % _k;
  int east_node;
  // if at the right edge of the dimension, wraparound to left
  if ( loc_in_dim == ( _k-1 ) ) {
    east_node = node - (_k-1)*k_to_dim;
  } else {
    east_node = node + k_to_dim;
  }
  return east_node;
}

int UnidirectionalTorus::_SouthNode( int node, int dim )
{
  int k_to_dim = powi( _k, dim );
  int loc_in_dim = ( node / k_to_dim ) % _k;
  int south_node;
  // if at the bottom edge of the dimension, wraparound to top
  if ( loc_in_dim == 0 ) {
    south_node = node + (_k-1)*k_to_dim;
  } else {
    south_node = node - k_to_dim;
  }
  return south_node;
}

int UnidirectionalTorus::_WestNode( int node, int dim )
{
  int k_to_dim = powi( _k, dim );
  int loc_in_dim = ( node / k_to_dim ) % _k;
  int west_node;
  // if at the left edge of the dimension, wraparound to right
  if ( loc_in_dim == 0 ) {
    west_node = node + (_k-1)*k_to_dim;
  } else {
    west_node = node - k_to_dim;
  }
  return west_node;
}

int UnidirectionalTorus::_NorthNode( int node, int dim )
{
  int k_to_dim = powi( _k, dim );
  int loc_in_dim = ( node / k_to_dim ) % _k;
  int north_node;
  // if at the top edge of the dimension, wraparound to bottom
  if ( loc_in_dim == ( _k-1 ) ) {
    north_node = node - (_k-1)*k_to_dim;
  } else {
    north_node = node + k_to_dim;
  }
  return north_node;
}

int UnidirectionalTorus::GetN( ) const
{
  return _n;
}

int UnidirectionalTorus::GetK( ) const
{
  return _k;
}

double UnidirectionalTorus::Capacity( ) const
{
  return 1.0; // Same as bidirectional torus
}

void UnidirectionalTorus::InsertRandomFaults( const Configuration &config )
{
  // Implementation for fault insertion (similar to KNCube)
  // For now, just use the default implementation
  Network::InsertRandomFaults( config );
}

 
