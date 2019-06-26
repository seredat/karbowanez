// Copyright (c) 2016, Dan Boneh, Henry Corrigan-Gibbs, Stuart Schechter
// Copyright (c) 2019, itwysgsl
// Copyright (c) 2019, The Karbo developers
//
// This file is part of Karbo.
//
// Karbo is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Karbo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Karbo.  If not, see <http://www.gnu.org/licenses/>.

// https://github.com/itwysgsl/balloon/


#ifndef BALLOON_S_H
#define BALLOON_S_H

#ifdef __cplusplus
extern "C" {
#endif

void balloon_s(const unsigned char* input, char* output, int length, const unsigned char* salt, int salt_length);

#ifdef __cplusplus
}
#endif

#endif
