#!/usr/bin/env python3
"""Generate Terraduino's compact enum string decoders from TerraStrings.cpp."""
from __future__ import annotations
import argparse, functools, re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
SOURCE=ROOT/'src'/'TerraStrings.cpp'
OUTPUT=ROOT/'src'/'TerraEnumTrie.h'
SPECS=[
    ("terraObjectTypeToString", "Terra_ObjectType", "terraDecodeObjectType", "objectTypeStr", "Terra_ObjectType_Undefined"),
    ("TerraReservoirTypeToString", "Terra_ReservoirType", "terraDecodeReservoirType", "ReservoirTypeStr", "Terra_ReservoirType_Undefined"),
    ("TerraReservoirStateToString", "Terra_ResourceState", "terraDecodeResourceState", "resourceStateStr", "Terra_ResourceState_Unknown"),
    ("terraWaterSourceTypeToString", "Terra_WaterSourceType", "terraDecodeWaterSourceType", "waterSourceTypeStr", "Terra_WaterSourceType_Undefined"),
    ("terraWaterStorageTypeToString", "Terra_WaterStorageType", "terraDecodeWaterStorageType", "waterStorageTypeStr", "Terra_WaterStorageType_Undefined"),
    ("terraSensorTypeToString", "Terra_SensorType", "terraDecodeSensorType", "sensorTypeStr", "Terra_SensorType_Undefined"),
    ("terraActuatorTypeToString", "Terra_ActuatorType", "terraDecodeActuatorType", "actuatorTypeStr", "Terra_ActuatorType_Undefined"),
    ("terraMeasurementModeToString", "Terra_MeasurementMode", "terraDecodeMeasurementMode", "measurementModeStr", "Terra_MeasurementMode_Undefined"),
    ("terraEnableModeToString", "Terra_EnableMode", "terraDecodeEnableMode", "enableModeStr", "Terra_EnableMode_Undefined"),
    ("terraPinModeToString", "Terra_PinMode", "terraDecodePinMode", "pinModeStr", "Terra_PinMode_Undefined"),
    ("terraUnitToString", "Terra_Unit", "terraDecodeUnit", "unitStr", "Terra_Unit_Undefined"),
    ("terraUnitsCategoryToString", "Terra_UnitsCategory", "terraDecodeUnitsCategory", "unitsCategoryStr", "Terra_UnitsCategory_Undefined"),
    ("terraRailTypeToString", "Terra_RailType", "terraDecodeRailType", "railTypeStr", "Terra_RailType_Undefined"),
    ("terraComparisonToString", "Terra_Comparison", "terraDecodeComparison", "comparisonStr", "Terra_Comparison_GreaterOrEqual"),
    ("terraTriggerStateToString", "Terra_TriggerState", "terraDecodeTriggerState", "triggerStateStr", "Terra_TriggerState_Fault"),
    ("terraLogLevelToString", "Terra_LogLevel", "terraDecodeLogLevel", "logLevelStr", "Terra_LogLevel_Info"),
    ("terraModuleTypeToString", "Terra_ModuleType", "terraDecodeModuleType", "moduleTypeStr", "Terra_ModuleType_Undefined"),
    ("terraControlModeToString", "Terra_ControlMode", "terraDecodeControlMode", "controlModeStr", "Terra_ControlMode_Disabled"),
    ("terraRouteStateToString", "Terra_RouteState", "terraDecodeRouteState", "routeStateStr", "Terra_RouteState_Idle"),
    ("terraAttachmentRoleToString", "Terra_AttachmentRole", "terraDecodeAttachmentRole", "attachmentRoleStr", "Terra_AttachmentRole_Undefined"),
]

def function_body(source,name):
    start=source.index(f'TerraString {name}('); brace=source.index('{',start); depth=1; pos=brace+1
    while depth and pos<len(source):
        depth += (source[pos]=='{') - (source[pos]=='}'); pos+=1
    if depth: raise RuntimeError(f'Unable to parse {name}')
    return source[brace+1:pos-1]

def string_values(source):
    vals={}
    pat=re.compile(r'case\s+(TStr_[A-Za-z0-9_]+):\s*\{\s*static const char flashStr\[\] PROGMEM = "([^"\\]*(?:\\.[^"\\]*)*)";',re.S)
    for ident,escaped in pat.findall(source): vals[ident]=bytes(escaped,'utf-8').decode('unicode_escape')
    return vals

def values_for(source,name):
    body=function_body(source,name); strings=string_values(source); vals=[]
    for enum_name,string_id in re.findall(r'case\s+(Terra_[A-Za-z0-9_]+):\s*return\s+SFP\((TStr_[A-Za-z0-9_]+)\);',body):
        vals.append((strings[string_id],enum_name))
    if not vals: raise RuntimeError(f'No enum values parsed for {name}')
    return tuple(vals)

def char_at(text,pos): return text[pos].lower() if pos<len(text) else '\0'

def pick_tree(values):
    max_len=max(len(t) for t,_ in values)
    @functools.lru_cache(maxsize=None)
    def solve(indices,positions):
        if len(indices)==1: return (1,('leaf',indices[0]))
        best=None
        for pos in positions:
            groups={}
            for i in indices: groups.setdefault(char_at(values[i][0],pos),[]).append(i)
            if len(groups)<=1: continue
            remaining=tuple(p for p in positions if p!=pos); children=[]; cost=2+len(groups); valid=True
            for ch,group in sorted(groups.items()):
                child_cost,child=solve(tuple(group),remaining)
                if child is None: valid=False; break
                cost+=child_cost; children.append((ch,child))
            if valid:
                cand=(cost,pos,('switch',pos,tuple(children)))
                if best is None or cand[:2]<best[:2]: best=cand
        return (10**9,None) if best is None else (best[0],best[2])
    return solve(tuple(range(len(values))),tuple(range(max_len+1)))[1]

def cpp_char(ch):
    if ch=='\0': return "'\\0'"
    if ch=="'": return "'\\\''"
    if ch=='\\': return "'\\\\'"
    return repr(ch)

def emit(tree,values,var,default,indent=1):
    pad='    '*indent
    if tree[0]=='leaf':
        _,enum_name=values[tree[1]]
        return [f'{pad}return {enum_name};']
    _,pos,children=tree
    lines=[f'{pad}switch (terraTrieChar({var}, {pos})) {{']
    for ch,child in children:
        lines.append(f'{pad}    case {cpp_char(ch)}:')
        lines.extend(emit(child,values,var,default,indent+2))
    lines += [f'{pad}}}',f'{pad}return {default};']
    return lines

def generate():
    source=SOURCE.read_text()
    lines=['/*  Terraduino enum decoding tree.','    Generated by tests/generate_enum_trie.py. Do not hand edit.','*/','','#ifndef TerraEnumTrie_H','#define TerraEnumTrie_H','']
    for fn,etype,decoder,var,default in SPECS:
        vals=values_for(source,fn); tree=pick_tree(vals)
        if tree is None: raise RuntimeError(f'Unable to build {fn}')
        lines += [f'inline {etype} {decoder}(const TerraString &{var})','{']
        lines.extend(emit(tree,vals,var,default)); lines += ['}','']
    lines += ['#endif // /ifndef TerraEnumTrie_H','']
    return '\n'.join(lines)

def main():
    p=argparse.ArgumentParser(); p.add_argument('--check',action='store_true'); args=p.parse_args(); out=generate()
    if args.check:
        cur=OUTPUT.read_text() if OUTPUT.exists() else ''
        if cur!=out:
            print('TerraEnumTrie.h is stale. Run tests/generate_enum_trie.py.'); return 1
        print('TerraEnumTrie.h matches generated enum decoder.'); return 0
    OUTPUT.write_text(out); print(f'Wrote {OUTPUT.relative_to(ROOT)} ({len(out)} bytes)'); return 0
if __name__=='__main__': raise SystemExit(main())
