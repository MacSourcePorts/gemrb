/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#ifndef MOSIMPORTER_H
#define MOSIMPORTER_H

#include "ImageMgr.h"

namespace GemRB {

class MOSImporter : public ImageMgr {
private:
	ieWord Cols = 0;
	ieWord Rows = 0;
	ieDword BlockSize = 0;
	ieDword PalOffset = 0;
public:
	MOSImporter() noexcept = default;
	bool Import(DataStream* stream) override;
	Holder<Sprite2D> GetSprite2D() override;
};

}

#endif
