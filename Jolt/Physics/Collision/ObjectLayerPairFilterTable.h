// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Physics/Collision/ObjectLayer.h>

JPH_NAMESPACE_BEGIN

/// Filter class to test if two objects can collide based on their object layer. Used while finding collision pairs.
/// This implementation uses a table to determine if two layers can collide.
class ObjectLayerPairFilterTable : public ObjectLayerPairFilter
{
private:
	/// Get which bit corresponds to the pair (inLayer1, inLayer2)
	uint					GetBit(ObjectLayer inLayer1, ObjectLayer inLayer2) const
	{
		// We store the lower left half only, so swap the inputs when trying to access the top right half
		if (inLayer1 > inLayer2)
			swap(inLayer1, inLayer2);

		JPH_ASSERT(inLayer2 < mNumObjectLayers);

		// Calculate at which bit the entry for this pair resides
		// We use the fact that a row always starts at inLayer2 * inLayer2 / 2
		// (this is the amount of bits needed to store a table of inLayer2 entries)
		return (inLayer2 * inLayer2) / 2 + inLayer1;
	}

public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructs the table with inNumObjectLayers Layers, initially all layer pairs are disabled
	explicit				ObjectLayerPairFilterTable(uint inNumObjectLayers) :
		mNumObjectLayers(inNumObjectLayers)
	{
		// By default everything collides
		int table_size = (inNumObjectLayers * inNumObjectLayers / 2 + 7) / 8;
		mTable.resize(table_size, 0);
	}

	/// Get the number of object layers
	uint					GetNumObjectLayers() const
	{
		return mNumObjectLayers;
	}

	/// Disable collision between two object layers
	void					DisableCollision(ObjectLayer inLayer1, ObjectLayer inLayer2)
	{
		uint bit = GetBit(inLayer1, inLayer2);
		mTable[bit >> 3] &= (0xff ^ (1 << (bit & 0b111)));
	}

	/// Enable collision between two object layers
	void					EnableCollision(ObjectLayer inLayer1, ObjectLayer inLayer2)
	{
		uint bit = GetBit(inLayer1, inLayer2);
		mTable[bit >> 3] |= 1 << (bit & 0b111);
	}

	/// Returns true if two layers can collide
	virtual bool			ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
	{
		// Test if the bit is set for this group pair
		uint bit = GetBit(inObject1, inObject2);
		return (mTable[bit >> 3] & (1 << (bit & 0b111))) != 0;
	}

private:
	uint					mNumObjectLayers;							///< The number of layers that this table supports
	Array<uint8>			mTable;										///< The table of bits that indicates which layers collide
};

JPH_NAMESPACE_END