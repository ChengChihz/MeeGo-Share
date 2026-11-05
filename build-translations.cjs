const fs=require('fs');
const keys=JSON.parse(fs.readFileSync(__dirname+'/i18n-keys.json','utf8')),rows=require('./translations-data.cjs');
if(rows.length!==keys.length)throw Error(`Translation row mismatch ${rows.length}/${keys.length}`);
const escape=s=>s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;');
fs.mkdirSync(__dirname+'/translations',{recursive:true});
for(const [i,lang] of ['en','zh_TW','ja','ko'].entries()){
 let xml='<?xml version="1.0" encoding="utf-8"?>\n<!DOCTYPE TS>\n<TS version="2.0" language="'+lang+'" sourcelanguage="zh_CN"><defaultcodec>UTF-8</defaultcodec><context><name>App</name>\n';
 keys.forEach((key,n)=>{const value=rows[n][i];if(!value||rows[n].length!==4)throw Error('Missing translation '+key);const placeholders=x=>(x.match(/%\d+/g)||[]).sort().join(',');if(placeholders(key)!==placeholders(value))throw Error('Placeholder mismatch '+lang+' '+key);xml+='<message utf8="true"><source>'+escape(key)+'</source><translation>'+escape(value)+'</translation></message>\n';});
 xml+='</context></TS>\n';fs.writeFileSync(__dirname+'/translations/app_'+lang+'.ts',xml);
}
console.log({messages:keys.length,languages:['zh_CN','zh_TW','en','ja','ko'],allTranslationsComplete:true});

const probe={keys,zh_CN:keys};for(const [i,l] of ["en","zh_TW","ja","ko"].entries())probe[l]=rows.map(r=>r[i]);fs.writeFileSync(__dirname+"/translations/probe.json",JSON.stringify(probe));

