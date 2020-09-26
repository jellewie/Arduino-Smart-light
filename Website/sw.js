self.addEventListener("install", e => {
	e.waitUntil(onInstall());
});

async function onInstall(){
	await self.skipWaiting();
}

self.addEventListener("fetch", e => {
	let respPromise = (async _ => {
		let resp = null;
		try{
			resp = await fetch(e.request);
		}catch(e){}
		if(resp && resp.ok){
			let cache = await openCache();
			cache.put(e.request, resp.clone());
			return resp;
		}else{
			const cacheValue = await caches.match(e.request);
			console.log(cacheValue);
			if(cacheValue) return cacheValue;
			return resp;
		}
	})();
	e.respondWith(respPromise);
});

async function openCache(){
	return await caches.open("mainCache");
}
