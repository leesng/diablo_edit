#include "StdAfx.h"

#include <iterator>

#include "Diablo Edit2.h"
#include "D2S_Struct.h"

using namespace std;

//d2s文件的CRC算法
static DWORD ComputCRC(const BYTE* source, DWORD len, DWORD init) {
	ASSERT(source && len);
	for (UINT i = 0, add; i < len; ++i, init = (init << 1) + add)
		add = (init & 0x80000000 ? 1 : 0) + source[i];
	return init;
}

//检验数据的CRC，会修改数据内容
static BOOL ValidateCrc(std::vector<BYTE>& data, DWORD dwCrc, DWORD dwOff) {
	ASSERT(dwOff + 4 <= data.size());
	*reinterpret_cast<DWORD*>(&data[dwOff]) = 0;
	return (dwCrc == ::ComputCRC(&data[0], (DWORD)data.size(), 0));
}

//struct CQuestInfoData

void CQuestInfoData::ReadData(CInBitsStream& bs) {
	bs >> wIntroduced1 >> wActI >> wTraval1
		>> wIntroduced2 >> wActII >> wTraval2
		>> wIntroduced3 >> wActIII >> wTraval3
		>> wIntroduced4 >> wActIV >> wTraval4
		>> unknown1 >> wIntroduced5 >> unknown2
		>> wActV >> bResetStats >> unknown3
		>> unknown4;
}

void CQuestInfoData::WriteData(COutBitsStream& bs) const {
	bs << wIntroduced1 << wActI << wTraval1
		<< wIntroduced2 << wActII << wTraval2
		<< wIntroduced3 << wActIII << wTraval3
		<< wIntroduced4 << wActIV << wTraval4
		<< unknown1 << wIntroduced5 << unknown2
		<< wActV << bResetStats << unknown3
		<< unknown4;
}

//struct CQuestInfo

void CQuestInfo::ReadData(CInBitsStream& bs) {
	bs >> dwMajic >> dwActs >> wSize;
	if (dwMajic != 0x216F6F57 || wSize != 0x12A)
		throw ::theApp.MsgBoxInfo(13);
	for (auto& p : QIData)
		p.ReadData(bs);
}

void CQuestInfo::WriteData(COutBitsStream& bs) const {
	bs << DWORD(0x216F6F57) << dwActs << WORD(0x12A);
	for (const auto& p : QIData)
		p.WriteData(bs);
}

//struct CWaypointData

void CWaypointData::ReadData(CInBitsStream& bs) {
	bs >> unkown >> Waypoints >> unkown2;
}

void CWaypointData::WriteData(COutBitsStream& bs) const {
	bs << unkown << Waypoints << unkown2;
}

//struct CWaypoints

void CWaypoints::ReadData(CInBitsStream& bs) {
	bs >> wMajic >> unkown >> wSize;
	if (wMajic != 0x5357 || wSize != 0x50)
		throw ::theApp.MsgBoxInfo(14);
	for (auto& p : wp)
		p.ReadData(bs);
}

void CWaypoints::WriteData(COutBitsStream& bs) const {
	bs << WORD(0x5357) << unkown << WORD(0x50);
	for (const auto& p : wp)
		p.WriteData(bs);
}

//CPlayerStats

// Mod 5
static const DWORD PLAYER_STATS_BITS_COUNT_MOD5[CPlayerStats::ARRAY_SIZE] = {
	11,11,11,11,10, 8,21,21,21,21,21,21, 7,32,32,32,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Mod 6
static const DWORD PLAYER_STATS_BITS_COUNT_MOD6[CPlayerStats::ARRAY_SIZE] = {
	10,10,10,10,10, 8,22,23,22,23,21,22, 9,32,26,26,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Mod 7
static const DWORD PLAYER_STATS_BITS_COUNT_MOD7[CPlayerStats::ARRAY_SIZE] = {
	10,10,10,10,10, 8,21,21,21,21,21,21,10,32,25,25,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
// Mod 8
static const DWORD PLAYER_STATS_BITS_COUNT_MOD8[CPlayerStats::ARRAY_SIZE] = {
	21,21,21,21,10, 8,21,21,21,21,21,21, 7,32,25,25,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void CPlayerStats::ReadData(CInBitsStream& bs) {
	bs >> wMajic;
	if (wMajic != 0x6667)
		throw ::theApp.MsgBoxInfo(15);
	::ZeroMemory(m_adwValue, sizeof(m_adwValue));

	for (UINT Magic = 0, Bits = 0, i = 0; i < 0x10; i++) {
		// ST00 ~ ST0F
		if (i < 10) {
			Magic = (((UINT)'0' + i) << 24) | ((UINT)'0' << 16) | ((UINT)'T' << 8) | ((UINT)'S');
		} else {
			Magic = (((UINT)'A' + i - 10) << 24) | ((UINT)'0' << 16) | ((UINT)'T' << 8) | ((UINT)'S');
		}

		Bits = ::theApp.MiscMetaData(0, Magic);
		if (Bits != 0) {
			PLAYER_STATS_BITS_COUNT[i] = Bits;
		}
	}

	for (bs >> bits(iEnd, 9); bs.Good()/* && iEnd < size(m_adwValue)*/; bs >> bits(iEnd, 9)) {
		if (iEnd < 0x10) {
			bs >> bits(m_adwValue[iEnd], PLAYER_STATS_BITS_COUNT[iEnd]);
		}
		// 读取 mod  自定义数据
		else if (iEnd != 511) {
			UINT i = 0;

			bs.SeekBack(9 / 8, 9 % 8);
			iEnd = bs.FindBitoffset(0x6669, 9);
			for (i = 0; i < iEnd / 32; ++i){
				assert(0x10 + i < size(m_adwValue));
				PLAYER_STATS_BITS_COUNT[0x10 + i] = 32;
				bs >> bits(m_adwValue[0x10 + i], PLAYER_STATS_BITS_COUNT[0x10 + i]);
			}
			if (iEnd % 32) {
				assert(0x10 + i < size(m_adwValue));
				PLAYER_STATS_BITS_COUNT[0x10 + i] = iEnd % 32;
				bs >> bits(m_adwValue[0x10 + i], PLAYER_STATS_BITS_COUNT[0x10 + i]);
			}
		} else {
			break;
		}
	}
	bs.AlignByte();
}

void CPlayerStats::WriteData(COutBitsStream& bs) const {
	bs << WORD(0x6667);
	for (UINT i = 0; bs.Good() && i < size(m_adwValue); ++i) {
		if (m_adwValue[i] && i < 0x10) {
			bs << bits(WORD(i), 9) << bits(m_adwValue[i], PLAYER_STATS_BITS_COUNT[i]);
		}
		// 写入 mod 自定义数据
		if (PLAYER_STATS_BITS_COUNT[i] && i >= 0x10) {
			bs << bits(m_adwValue[i], PLAYER_STATS_BITS_COUNT[i]);
		}
	}
	bs << bits<WORD>(0x1FF, 9);
	bs.AlignByte();
}
/*
void CPlayerStats::ReadData(CInBitsStream& bs) {
	bs >> wMajic;
	if (wMajic != 0x6667)
		throw ::theApp.MsgBoxInfo(15);
	::ZeroMemory(m_adwValue, sizeof(m_adwValue));
	::ZeroMemory(m_awIndex, sizeof(m_awIndex));
	WORD iEnd;
	DWORD iBits;
	for (UINT i = 0, j = 0x10; bs.Good() && i < size(m_adwValue); ++i) {
		bs >> bits(iEnd, 9);
		if (iEnd < 0x10) {
			m_awIndex[iEnd] = iEnd;
			iBits = ::theApp.m_nModIndex == 5 ? PLAYER_STATS_BITS_COUNT_MOD5[iEnd] : PLAYER_STATS_BITS_COUNT[iEnd];
			bs >> bits(m_adwValue[iEnd], iBits);
		} else {
			m_awIndex[j] = iEnd;
			if (iEnd == 0x1FF) {
				break;
			}
			iBits = ::theApp.m_nModIndex == 5 ? PLAYER_STATS_BITS_COUNT_MOD5[j] : PLAYER_STATS_BITS_COUNT[j];
			bs >> bits(m_adwValue[j], iBits);
			j++;
		}
	}

	bs.AlignByte();
}

void CPlayerStats::WriteData(COutBitsStream& bs) const {
	bs << WORD(0x6667);
	DWORD iBits;
	for (UINT i = 0; bs.Good() && i < size(m_adwValue); ++i) {
		if (m_awIndex[i] == 0x1FF) {
			bs << bits(m_awIndex[i], 9);
			break;
		}
		if (m_adwValue[i]) {
			if (i < 0x10) {
				bs << bits(WORD(i), 9);
				iBits = ::theApp.m_nModIndex == 5 ? PLAYER_STATS_BITS_COUNT_MOD5[i] : PLAYER_STATS_BITS_COUNT[i];
				bs << bits(m_adwValue[i], iBits);
			} else {
				bs << bits(m_awIndex[i], 9);
				iBits = ::theApp.m_nModIndex == 5 ? PLAYER_STATS_BITS_COUNT_MOD5[i] : PLAYER_STATS_BITS_COUNT[i];
				bs << bits(m_adwValue[i], iBits);
			}
		}
	}
	//bs << bits<WORD>(0x1FF, 9);
	bs.AlignByte();
}
*/
//struct CCharSkills

void CCharSkills::ReadData(CInBitsStream& bs) {
	bs >> wMagic;
	if (wMagic != 0x6669)
		throw ::theApp.MsgBoxInfo(16);

	for (UINT i = 0; bs.Good() && i < size(bSkillLevel); ++i) {
		bs >> bSkillLevel[i];
		if (i > 0 && bSkillLevel[i] == 0x4D && bSkillLevel[i-1] == 0x4A) { // NextMagic
			bs.SeekBack(sizeof(WORD));
			break;
		}
	}
}

void CCharSkills::WriteData(COutBitsStream& bs) const {
	bs << WORD(0x6669);
	for (UINT i = 0; bs.Good() && i < size(bSkillLevel); ++i) {
		if (i+1 < size(bSkillLevel) && bSkillLevel[i+1] == 0x4D && bSkillLevel[i] == 0x4A) { // NextMagic
			break;
		}
		bs << bSkillLevel[i];
	}
}

//struct CCorpseData

void CCorpseData::ReadData(CInBitsStream& bs, DWORD version) {
	bs >> unknown;
	stItems.ReadData(bs, version);
}

void CCorpseData::WriteData(COutBitsStream& bs, DWORD version) const {
	bs << unknown;
	stItems.WriteData(bs, version);
}

//struct CCorpse

void CCorpse::ReadData(CInBitsStream& bs, DWORD version) {
	bs >> wMagic >> wCount;
	if (wMagic != 0x4D4A || wCount > 1)
		throw ::theApp.MsgBoxInfo(19);
	if (wCount)
		pCorpseData.ensure().ReadData(bs, version);
}

void CCorpse::WriteData(COutBitsStream& bs, DWORD version) const {
	bs << WORD(0x4D4A);
	if (pCorpseData.exist()) {
		bs << WORD(1);
		pCorpseData->WriteData(bs, version);
	}
	else
		bs << WORD(0);
}

//struct CMercenary

void CMercenary::ReadData(CInBitsStream& bs, BOOL hasMercenary, DWORD version) {
	bs >> wMagic;
	if (wMagic != 0x666A)
		throw ::theApp.MsgBoxInfo(20);
	if (hasMercenary)
		stItems.ensure().ReadData(bs, version);
}

void CMercenary::WriteData(COutBitsStream& bs, BOOL hasMercenary, DWORD version) const {
	bs << WORD(0x666A);
	if (hasMercenary)
		stItems->WriteData(bs, version);
}

//struct CGolem

void CGolem::ReadData(CInBitsStream& bs, DWORD version) {
	bs >> wMagic >> bHasGolem;
	if (wMagic != 0x666B)
		throw ::theApp.MsgBoxInfo(21);
	if (bHasGolem)
		pItem.ensure().ReadData(bs, version);
}

void CGolem::WriteData(COutBitsStream& bs, DWORD version) const {
	bs << WORD(0x666B) << bHasGolem;
	if (bHasGolem)
		pItem->WriteData(bs, version);
}

//CD2S_Struct

void CD2S_Struct::ReadFile(const CString& path)
{
	CInBitsStream bs;
	bs.ReadFile(CFile(path, CFile::modeRead));
	ReadData(bs);
}

void CD2S_Struct::WriteFile(const CString& path) const {
	COutBitsStream bs;
	if (!WriteData(bs))
		return;
	bs.WriteFile(CFile(path, CFile::modeCreate | CFile::modeWrite));
}

void CD2S_Struct::Reset() {
	QuestInfo.Reset();
	Waypoints.Reset();
	PlayerStats.Reset();
	Skills.Reset();
	ItemList.Reset();
	stCorpse.Reset();
	stMercenary.Reset();
	stGolem.Reset();
}

void CD2S_Struct::ReadData(CInBitsStream& bs) {
	Reset();
	//得到人物信息
	bs >> dwMajic;
	if (dwMajic != 0xAA55AA55)
		throw ::theApp.MsgBoxInfo(11);
	bs >> dwVersion >> dwSize;
	if (bs.DataSize() != dwSize)
		throw ::theApp.MsgBoxInfo(12);
	const DWORD offCrc = bs.BytePos();
	bs >> dwCRC;
	if (!::ValidateCrc(bs.Data(), dwCRC, offCrc))	//校验CRC，会修改bs数据内容
		throw ::theApp.MsgBoxInfo(11);
	bs >> dwWeaponSet
		>> Name
		>> charType
		>> charTitle
		>> unkown1
		>> charClass
		>> unkown2
		>> charLevel
		>> unkown3
		>> dwTime
		>> unkown4
		>> dwSkillKey
		>> dwLeftSkill1
		>> dwRightSkill1
		>> dwLeftSkill2
		>> dwRightSkill2
		>> outfit
		>> colors
		>> Town
		>> dwMapSeed
		>> unkown5
		>> bMercDead
		>> unkown6
		>> dwMercControl
		>> wMercName
		>> wMercType
		>> dwMercExp
		>> unkown7
		>> NamePTR
		>> unkown8;
	QuestInfo.ReadData(bs);
	Waypoints.ReadData(bs);
	bs >> NPC;
	PlayerStats.ReadData(bs);
	Skills.ReadData(bs);
	ItemList.ReadData(bs, dwVersion);
	stCorpse.ReadData(bs, dwVersion);
	if (isExpansion()) {
		stMercenary.ReadData(bs, HasMercenary(), dwVersion);
		stGolem.ReadData(bs, dwVersion);
	}
	bs.AlignByte();
	if (!bs.Good() || bs.DataSize() != bs.BytePos())
		throw ::theApp.MsgBoxInfo(11);
}

BOOL CD2S_Struct::WriteData(COutBitsStream& bs) const {
	// Character data
	bs << DWORD(0xAA55AA55)	//dwMajic
		<< dwVersion;
	const DWORD offSize = bs.BytePos();
	bs << DWORD(0);			//data size
	const DWORD offCrc = bs.BytePos();
	bs << DWORD(0)			//CRC
		<< dwWeaponSet
		<< Name
		<< charType
		<< charTitle
		<< unkown1
		<< charClass
		<< unkown2
		<< charLevel
		<< unkown3
		<< dwTime
		<< unkown4
		<< dwSkillKey
		<< dwLeftSkill1
		<< dwRightSkill1
		<< dwLeftSkill2
		<< dwRightSkill2
		<< outfit
		<< colors
		<< Town
		<< dwMapSeed
		<< unkown5
		<< bMercDead
		<< unkown6
		<< dwMercControl
		<< wMercName
		<< wMercType
		<< dwMercExp
		<< unkown7
		<< NamePTR
		<< unkown8;
	QuestInfo.WriteData(bs);
	Waypoints.WriteData(bs);
	bs << NPC;
	PlayerStats.WriteData(bs);
	Skills.WriteData(bs);
	ItemList.WriteData(bs, dwVersion);
	stCorpse.WriteData(bs, dwVersion);
	if (isExpansion()) {
		stMercenary.WriteData(bs, HasMercenary(), dwVersion);
		stGolem.WriteData(bs, dwVersion);
	}
	bs.AlignByte();
	//Data size
	bs << offset_value(offSize, bs.BytePos());
	//CRC
	auto& data = bs.Data();
	const DWORD dwCrc = ::ComputCRC(&data[0], (DWORD)data.size(), 0);
	bs << offset_value(offCrc, dwCrc);
	return bs.Good();
}

void CD2S_Struct::name(const CString& name) {
	CStringA utf8 = EncodeCharName(name);
	const BOOL isPtr24 = IsPtr24AndAbove(dwVersion);
	// Copy data
	BYTE* dest = isPtr24 ? NamePTR : Name;
	int destLen = isPtr24 ? sizeof NamePTR : sizeof Name;
	::ZeroMemory(dest, destLen);
	::CopyMemory(dest, utf8, min(utf8.GetLength(), destLen));
}
