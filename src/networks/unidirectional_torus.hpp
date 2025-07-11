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

#ifndef _UNIDIRECTIONAL_TORUS_HPP_
#define _UNIDIRECTIONAL_TORUS_HPP_

#include "network.hpp"

class UnidirectionalTorus : public Network {

  int _k;
  int _n;

  void _ComputeSize( const Configuration &config );
  void _BuildNet( const Configuration &config );

  int _EastChannel( int node, int dim );
  int _SouthChannel( int node, int dim );

  int _EastNode( int node, int dim );
  int _SouthNode( int node, int dim );
  int _WestNode( int node, int dim );
  int _NorthNode( int node, int dim );

public:
  UnidirectionalTorus( const Configuration &config, const string & name );
  static void RegisterRoutingFunctions();

  int GetN( ) const;
  int GetK( ) const;

  double Capacity( ) const;

  void InsertRandomFaults( const Configuration &config );

};

#endif 
