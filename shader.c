///  My 4K Shader intro... for Fun - not profit !! 
//  Missing something on the blue screen ;-)
//   Harley  //
#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

#define size resolution
bool bColoring=false;//i luv globals
vec3 mcol;

const float mr=0.25, mxr=1.0;
const vec4 scale=vec4(-3.12,-3.12,-3.12,3.12),p0=vec4(0.0,1.59,-1.0,0.0);
float DE(in vec3 z0){//amazing box by tglad
	vec4 z = vec4(z0,1.0);
	for (int n = 0; n < 3; n++) {
		z.xyz=clamp(z.xyz, -0.94, 0.94)*2.0-z.xyz;
		z*=scale/clamp(dot(z.xyz,z.xyz),mr,mxr);
		z+=p0;
	}
	if(bColoring)mcol+=z.xyz;
	z.y-=3.0*sin(time*3.0+floor(z0.x+0.5)+floor(z0.z+0.5));
	float dS=(length(max(abs(z.xyz)-vec3(1.2,49.0,1.4),0.0))-0.06)/z.w;
	return dS;
}
float DEL(in vec3 z0){//amazing box by tglad
	vec4 z = vec4(z0,1.0),p0=vec4(0.0,1.28,-1.12,0.0);
	z.xyz=clamp(z.xyz, -0.94, 0.94)*2.0-z.xyz;
	z*=scale/clamp(dot(z.xyz,z.xyz),mr,mxr);
	return (length(z.xyz+vec3(0.0,5.8,2.2))-0.6)/z.w;
}
float mdl;
float MDE(in vec3 z0){
	float dL=DEL(z0);
	float dS=DE(z0);
	mdl=min(mdl,dL);
	return min(dS,dL);
}
float rndStart(vec2 co){return 0.8+0.2*fract(sin(dot(co,vec2(123.42,117.853)))*412.453);}
float ShadAO(vec3 ro, vec3 rd, float px, float dist){//pretty much IQ's SoftShadow
	float res=1.0,d,t=4.0*px*rndStart(gl_FragCoord.xy);
	for(int i=0;i<10;i++){
		d=max(0.0,DE(ro+rd*t))+0.01;
		if(t+d>dist)break;
		res=min(res,2.0*d/t);
		t+=d;
	}
	return res;
}
mat3 lookat(vec3 fw,vec3 up){
	fw=normalize(fw);vec3 rt=normalize(cross(fw,up));return mat3(rt,cross(rt,fw),fw);
}
const vec3 light_col=vec3(1.0,0.7,0.4);
vec3 Light(vec3 so, vec3 rd, float px, float dist){
	vec2 v=vec2(px,0.0);//px is really pixelSize*t
	mcol=vec3(0.0);
	so+=rd*(dist-px);
	bColoring=true;//take color samples
	vec3 col,norm=normalize(vec3(-DE(so-v.xyy)+DE(so+v.xyy),
		-DE(so-v.yxy)+DE(so+v.yxy),-DE(so-v.yyx)+DE(so+v.yyx)));
	bColoring=false;//crappy lighting below
	mcol=vec3(0.9)+sin(mcol)*0.1;
	float dL=DEL(so);
	if(dL>.0*px){
		v=vec2(dL,0.0);
		vec3 light_dir=-normalize(vec3(-DEL(so-v.xyy)+DEL(so+v.xyy),-DEL(so-v.yxy)+DEL(so+v.yxy),-DEL(so-v.yyx)+DEL(so+v.yyx)));
		light_dir=normalize(light_dir+vec3(0.0,1.0,0.0));
		vec3 diffuse_col=mcol+vec3(-0.125,0.0,0.125)*dot(norm,rd);
		float shad=ShadAO(so,light_dir,px,dL);
		float dif=dot(norm,light_dir)*0.5+0.5;
		float spec=0.25*pow(max(0.0,dot(light_dir,reflect(rd,norm))),0.25);
		dif=min(dif,shad);
		col=diffuse_col*dif+light_col*spec*shad;
		col/=(1.0+dL*dL);
	}else {col=light_col*1.5;}
	return col;
}
float hash( float n ){return fract(sin(n)*43758.5453);}
float hash( vec2 n ){return fract(sin(dot(n*0.123,vec2(78.233,113.16)))*43758.351);}
float noise(in float p){
	float c=floor(p),f=fract(p);
	return mix(hash(c),hash(c+1.0),f);
}
float noise(in vec2 p){
	vec2 c=floor(p),f=fract(p),v=vec2(1.0,0.0);
	return mix(mix(hash(c),hash(c+v),f.x),
		mix(hash(c+v.yx),hash(c+v.xx),f.x),f.y);
}
void main(){
	float zoom=1.5,px=2.0/(size.y*zoom);//find the pixel size
	float tim=time;
	
	//position camera
	vec3 ro=vec3(sin(tim*0.1),0.0,cos(tim*0.05));
	ro.z=0.25+ro.z*abs(ro.z);
	ro*=3.9;
	if(abs(ro.x)>3.8)ro.x=3.8*sign(ro.x);
	if(abs(ro.z)>3.9)ro.z=3.9*sign(ro.z);
	vec3 rd=normalize(vec3((2.0*gl_FragCoord.xy-size.xy)/size.y,zoom));
	rd=lookat(vec3(0.25+sin(tim*0.6),sin(tim*0.4),0.25)-ro,vec3(0.01,0.99,0.02))*rd;
	
	//march
	float t=rndStart(gl_FragCoord.xy)*0.5,tt=t,d,dm=100.0,tm=0.0,od=1000.0;
	float ft=(sign(rd.y)-ro.y)/rd.y,ref=1.0,dR=clamp(DE(ro+rd*ft)*10.0,0.0,1.0);
	float maxT=min((sign(rd.x)*4.0-ro.x)/rd.x,(sign(rd.z)*4.0-ro.z)/rd.z);
	bool bGrab=false;
	float stepGlow=0.0;
	mdl=100.0;
	for(int i=0;i<48;i++){
		d=MDE(ro+rd*t);
		t+=d;tt+=d;stepGlow+=0.006;
		if(t>ft){
			ro+=rd*ft;
			t=t-ft;//the overshoot
			if(tt-t<maxT){//hit floor/ceiling
				vec2 p=mod(2.0*vec2(ro.x+ro.z,ro.x-ro.z),2.0)-1.0;
				float tile=sign(p.x*p.y);
				if(tile>0.0){
					rd.y=-rd.y;
					ft=(sign(rd.y)-ro.y)/rd.y;
					ref*=0.75;
				}else{
					tt+=1000.0;
					break;
				}
			}else{//hit wall
				t=maxT-tt+t;
				ro+=rd*t;
				break;
			}
		}else if(d>od){
			if(bGrab && od<px*tt && tm==0.0){
				dm=od;
				tm=tt-d-od;
				bGrab=false;
			}
		}else bGrab=true;
		od=d;
		if(tt>maxT){
			t-=tt-maxT;
			ro=ro+rd*t;
			break;
		}
		if(d<0.00001)break;//hard stop
	}
	
	//color
	vec3 col=vec3(0.02,0.0,0.0);
	if(tt<maxT){
		if(d<px*t){
			col=mix(Light(ro+rd*t,rd,px*t,d),col,clamp(d/(px*t),0.0,1.0));
		}
		if(dm<px*tm){
			col=mix(Light(ro+rd*tm,rd,px*tm,dm),col,clamp(dm/(px*tm),0.0,1.0));
		}
	}else{
		if(tt>1000.0){
			vec2 p=mod(2.0*vec2(ro.x+ro.z,ro.x-ro.z),2.0)-1.0;
			float g=max(0.15,pow(1.55*max(abs(abs(p.x)-0.5),abs(abs(p.y)-0.5)),6.0));
			col=vec3(dR*g);
			tt-=1000.0;
		}else{
			tt=maxT;
			if(abs(ro.z)>abs(ro.x))ro.xz=ro.zx;
			d=noise(ro.yz*70.0);
			ro.y*=4.0;
			od=max(abs(ro.z),abs(ro.y))-1.0;
			dm=pow(1.0-clamp(abs(sin(time*10.0+ro.z*150.0*sin(time))+ro.y*1.2),0.0,1.0),10.0);
			ro.y+=0.5;
			ro.z+=floor(mod(ro.y+0.5,2.0))*0.25;
			col=vec3(0.2,0.15,0.1)*(1.0-0.5*exp(-200.0*abs((fract(ro.z*2.0)-0.5)*(fract(ro.y)-0.5))));
			col-=d*vec3(0.1,0.05,0.0);
			col=mix(vec3(0.0,dm,1.0)*clamp(abs(od*2.0),0.0,0.5),col,clamp(od*10.0,0.0,1.0));
		}
	}
	d=noise(time*10.0+rd.x*rd.z);
	col=mix(vec3(dR),pow(col,vec3(ref)),ref);
	col+=light_col*exp(-mdl*100.0*clamp(d,0.05,0.5));
	col+=vec3(0.4,0.8,1.0)*pow(stepGlow*clamp(4.0-d*100.0,1.0,4.0),3.0);
	col=3.0*col*exp(-tt*0.3);
	gl_FragColor=vec4(col,1.0);
} 

