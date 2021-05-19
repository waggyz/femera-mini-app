more off; format short; format compact;
graphics_toolkit("gnuplot");
PCT=0.01;
%
lws='linewidth' ; hlw= 0.25; flw= 1.5; lw = 2; mlw=1; tlw=4;
mss='markersize'; sms= 2; ms = 4; bms=5; gms=6;
fss='fontsize'  ; fs =11; bfs=12; tfs=10; lfs=7;
has='horizontalalignment';
vas='verticalalignment';
fas='fontangle';
%
ns=1e-9; us=1e-6; ms=1e-3; sc=1; mn=60; hr=60*mn; dy=24*hr;
%
figdir = '../build/tests/'
%
%
csv = dlmread ('perf/iters2solve-1e-5.csv');
%
if (0==1);
  figure (99); clf; hold on; grid on;
  loglog (csv(:,1),csv(:,2),'-bo','linewidth',3);
  f = 5:numel(csv(:,1));
  poly = polyfit (log(csv(f,1)),log(csv(f,2)), 1),
  loglog (csv(:,1),exp(polyval(poly,log(csv(:,1)))),'-r','linewidth',1);
  %poly3 = poly; poly3(1)=1/3; poly3,
  poly3 = [1/3,1.5],
  loglog (csv(:,1),exp(polyval(poly3,log(csv(:,1)))),'-m','linewidth',1);
  %loglog (csv(:,1),exp( log (csv(:,1))*1/3+1.5 ),'-k','linewidth',1);
  return
end;
%
csv = dlmread ('../build/tests/sm-md.csv');
%csv = dlmread ('../build/tests/sm-md-no-solve.csv');
%csv = dlmread ('../build/tests/sm-md-sleep.csv');
%
mesh_time = dlmread ('../build/tests/sm-md-time.csv');
%
start = min(csv(:,6));
stop  = max(csv(:,7));
%
elapsed = stop-start;
if (elapsed*ns < 360);
  xunit = 1/sc; xunitstr='s';
else;
  xunit = 1/mn; xunitstr='m';
end;
%
figure (1); clf; hold on; grid on;
axis ([0,elapsed*ns*xunit, -1.0,3.5]);
xlabel (['Elapsed time (',xunitstr,')']);
ylabel ('Physical CPU');
set (gca,'ytick',[0:3]);
barw=20;
%
chck = find (csv(:,5)==1);
scan = find (csv(:,5)==1.1);
info = find (csv(:,5)==1.2);
mesh = find (csv(:,5)==2);
prep = find (csv(:,5)==3);
iter = find (csv(:,5)==4);
zzzz = find (csv(:,5)==5);
post = find (csv(:,5)==10);
%
x = nan(3*numel(chck),1); y=x;
x(1:3:end) = (csv(chck,6)-start)*ns*xunit; y(1:3:end) = csv(chck,1);
x(2:3:end) = (csv(chck,7)-start)*ns*xunit; y(2:3:end) = csv(chck,1);
plot (x,y,'-k','linewidth',barw);
%
x = nan(3*numel(scan),1); y=x;
x(1:3:end) = (csv(scan,6)-start)*ns*xunit; y(1:3:end) = csv(scan,1);
x(2:3:end) = (csv(scan,7)-start)*ns*xunit; y(2:3:end) = csv(scan,1);
plot (x,y,'-y','linewidth',barw);
%
%x = nan(3*numel(info),1); y=x;
%x(1:3:end) = (csv(info,6)-start)*ns*xunit; y(1:3:end) = csv(info,1);
%x(2:3:end) = (csv(info,7)-start)*ns*xunit; y(2:3:end) = csv(info,1);
%plot (x,y,'-c','linewidth',barw);
%
x = nan(3*numel(mesh),1); y=x;
x(1:3:end) = (csv(mesh,6)-start)*ns*xunit; y(1:3:end) = csv(mesh,1);
x(2:3:end) = (csv(mesh,7)-start)*ns*xunit; y(2:3:end) = csv(mesh,1);
plot (x,y,'-m','linewidth',barw);
%
x = nan(3*numel(prep),1); y=x;
x(1:3:end) = (csv(prep,6)-start)*ns*xunit; y(1:3:end) = csv(prep,1);
x(2:3:end) = (csv(prep,7)-start)*ns*xunit; y(2:3:end) = csv(prep,1);
plot (x,y,'-b','linewidth',barw);
%
if (numel(iter)); solvstr='No Solve'; solvshort='no-solve';
  x = nan(3*numel(iter),1); y=x;
  x(1:3:end) = (csv(iter,6)-start)*ns*xunit; y(1:3:end) = csv(iter,1);
  x(2:3:end) = (csv(iter,7)-start)*ns*xunit; y(2:3:end) = csv(iter,1);
  plot (x,y,'-w','linewidth',barw);
end;
if (numel(zzzz)); solvstr='Sleep'; solvshort='sleep';
  x = nan(3*numel(zzzz),1); y=x;
  x(1:3:end) = (csv(zzzz,6)-start)*ns*xunit; y(1:3:end) = csv(zzzz,1);
  x(2:3:end) = (csv(zzzz,7)-start)*ns*xunit; y(2:3:end) = csv(zzzz,1);
  plot (x,y,'-r','linewidth',barw);
end;
%
x = nan(3*numel(post),1); y=x;
x(1:3:end) = (csv(post,6)-start)*ns*xunit; y(1:3:end) = csv(post,1);
x(2:3:end) = (csv(post,7)-start)*ns*xunit; y(2:3:end) = csv(post,1);
plot (x,y,'-c','linewidth',barw);
%
plot ((csv(:,6)-start)*ns*xunit,csv(:,1),'>g');
plot ((csv(:,7)-start)*ns*xunit,csv(:,1),'or');
%
legend (...
  'Check','Scan','Mesh','Prep',solvstr,'Post',...
  'Begin','Done',...
  'location','south', 'orientation','horizontal');
%
figure (1); hold on;
paper=[0.25,0.25, 6,3];
set(gcf,'paperposition',paper);
figname=['sm-md-gandt-',solvshort],
print([figdir,figname,'.eps'],'-depsc2','-FHelvetica');
print([figdir,figname,'.pdf'],'-depsc2','-FHelvetica');
%
%==============================================================================
figure (2); clf; hold on; grid on;
%
run_cncr = 4,
ref_cncr = 40,
%
set (gca, 'position',  [0.15,0.15, 0.75, 0.7]);
if (1==1);%(run_cncr == ref_cncr);
  s='performance';
else;
  s='estimate'
end;
title (['Batch ',s,' per Skylake node, 40 sims/40 cpu, ',...
  '|u_{ij}^{FEM}-u_{ij}^{exact}| <10^{-5}']);
axis ([1000,10e9, sc/10000 /10,3*dy]);
%
if (1==0)
  text (3e10,1,'Time','rotation',90,...
    'verticalalignment','top','horizontalalignment','center');
  y=10.^(-4:5);
  x=1.23e10*ones(size(y));
  ytic = ['10^{-4}';'10^{-3}';'10^{-2}';'10^{-1}';...
          '10^0   ';'10^1   ';'10^2   ';'10^3   ';'10^4   ';'10^5   '];
else;
  y=[100*us,1*ms,10*ms,100*ms,1*sc,6*sc,1*mn,6*mn,1*hr,1*dy,3*dy];
  x=1.23e10*ones(size(y));
  ytic = ['100 \mus';'1 ms';'10 ms';'100 ms';...
          '1 s   ';'6 s   ';'1 m   ';'6 m   ';'1 h   ';'1 d   ';'3 d   '];
end;
text (x,y,ytic);
%
xlabel ('3D linear-elastic simulation size (dof)');
if (run_cncr == ref_cncr);
  s='Expected';
else;
  s='Estimated'
end;
ylabel ([s,' throughput (sims/time)']);
%
set (gca,'xtick', 10.^[1e-9:11]);% hack around integer overflow for xtick 10^10
ytic = 1./[1/(3*dy),1/dy,1/hr,10/hr,1/mn,10/mn,1/sc,...
  10/sc,100/sc,1e3/sc,10e3/sc];
set (gca,'ytick', ytic);
ylab = ['1/3d';'1/d';'1/h';'10/h';'1/m';...
       '10/m';'1/s';'10/s';'100/s';'1k/s';'10k/s'];
set (gca,'yticklabel',ylab);
%
fr = sc/24; loglog ([1e3,1e10],[fr,fr],'--k', lws,hlw, mss,ms);
text (1e10,fr,'24 sims/s/node (Movie)','color','k',...
  has,'right',vas,'bottom', fss,lfs);
fr = sc/30; loglog ([1e3,1e10],[fr,fr],'--k', lws,hlw, mss,ms);
text (1e10,fr,'30 sims/s/node (NTSC)','color','k',...
  has,'right',vas,'top', fss,lfs);
%
mx = 1./(40/(3*dy));
ax = loglog ([1e3,10e9],[mx,mx],'-r', lws,flw);
text (1e7,mx,'40 sims/3 d/node','color','r',vas,'top', fss,lfs);
%
barw=3; ms =3;
%
encolor = ['b';'k';'m'];
sim_n = max (csv(:,2)),
tmin = 0;%0.100 *cncr/sim_n *0,%FIXME 100ms startup time, pro-rated
%
dofs_span = 10.^((3.0-1e-9):0.1:10);
f=find(csv(:,10)>0);
dofs_min=min(csv(f,10)), dofs_avg=mean(csv(f,10)), dofs_max=max(csv(f,10)),
%
prep = mesh_time * run_cncr / ref_cncr;
si = csv(:,2);
chck = zeros (sim_n,1); mesh=chck; read=chck; solv=chck; dofs=chck;
f = find (csv(:,10)>0); dofs(si(f)) = csv(f,10);
f = find (csv(:,5)==1); chck(si(f)) = tmin + (csv(f,7)-csv(f,6))*ns / ref_cncr;
f = find (csv(:,5)==2); mesh(si(f)) = tmin + (csv(f,7)-csv(f,6))*ns / ref_cncr;
f = find (csv(:,5)==3); read(si(f)) = tmin + (csv(f,7)-csv(f,6))*ns / ref_cncr;
f = find (csv(:,5)==5); solv(si(f)) = tmin + (csv(f,7)-csv(f,6))*ns / ref_cncr;
simt = prep+chck+mesh+read+solv;
prep(prep<=0)=nan;
chck(chck<=0)=nan;
mesh(mesh<=0)=nan;
read(read<=0)=nan;
solv(solv<=0)=nan;
%
w=ones(size(dofs));
%w(dofs>0)=1./log(dofs(dofs>0));% Fitting weights.

wdofs=dofs.*w; wprep=prep.*w; wchck=chck.*w; wmesh=mesh.*w; wread=read.*w;
wsolv=solv.*w;
%
f=find(mesh>0);%(geo,geo_unrolled)
poly_mesh_cads = polyfit (wdofs(f),wmesh(f),1),
poly_mesh_cads(end) = min(wmesh(f)),
%
s = unique(csv(find(csv(:,4)~=5),2)); s(s<=0)=[];% NOT cgns
fread = s;
poly_read = polyfit (wdofs(s),wread(s),1),
if (poly_read(end)<0);% Re-fit, assuming y-intercept is min(wread(fread).
  rmin = min(wread(fread));
  poly_read = polyfit (wdofs(fread),(wread(fread)-rmin)./wdofs(fread),0);
  %poly_read = [poly_read,0],
  poly_read = [poly_read,rmin],
end;
%
fcgns = unique( csv( find( csv(:,4)==5),2));% cgns
poly_read_cgns = polyfit (wdofs(fcgns),wread(fcgns),1),
poly_chck_cgns = polyfit (wdofs(fcgns),wchck(fcgns),0),
%poly_chck_cgns(end) = min(wchck(fcgns)),
%
fmesh = unique( csv( find( csv(:,4)~=1),2));% NOT CAD (geo,geo_unrolled)
fmesh(fmesh<=0)=[]; fmesh(prep(fmesh)<=0)=[];
poly_prep_mesh = polyfit (wdofs(fmesh),wprep(fmesh),1),
%poly_prep_mesh(end) = min(wprep(fmesh)),
%
fothr = unique( csv( find( (csv(:,4)~=1) & (csv(:,4)~=5)),2));% NOT CAD, NOT CGNS
fothr(fothr<=0)=[]; fothr(prep(fothr)<=0)=[];
if (1==1);% Assume y-intercept is min.
  cmin = min(wchck(fothr));
  poly_chck = polyfit (wdofs(fothr),(wchck(fothr)-cmin)./wdofs(fothr),0);
  poly_chck(2)=cmin;
else;
  poly_chck = polyfit (wdofs(fothr),wchck(fothr),1);
end;
%
fmshb = unique(csv(find(csv(:,3)==1 & csv(:,4)==4),2));% msh4 binary
poly_chck_msh4 = polyfit (wdofs(fmshb),wchck(fmshb),1),
if (poly_chck_msh4(end)<0);% Re-fit, assuming y-intercept is min.
  cmin = min(wchck(fmshb));
  poly_chck_msh4 = polyfit (wdofs(fmshb),(wchck(fmshb)-cmin)./wdofs(fmshb),0);
  poly_chck_msh4 = [poly_chck_msh4,cmin],
end;
%poly_read_mshb = polyfit (wdofs(fmshb),wread(fmshb),1),
%if (poly_read_mshb(end)<0);% Re-fit, assuming y-intercept is min.
%  rmin = min(wread(fmshb));
%  poly_read_mshb = polyfit (wdofs(fmshb),(wread(fmshb)-rmin)./wdofs(fmshb),0);
%  poly_read_mshb = [poly_read_mshb,cmin],
%end;
%
fcads = unique(csv(find(csv(:,4)==1),2));% CAD (geo,geo_unrolled)
fcads(fcads<=0)=[]; fcads(prep(fcads)<=0)=[];
%avg = mean(prep(fcads));
%fgeos = fcads(prep(fcads)<avg/2); fgeou=fcads(prep(fcads)>avg/2);
fgeou = unique(csv(find(csv(:,3)==2 & csv(:,4)==1),2));
fgeos = unique(csv(find(csv(:,3)==3 & csv(:,4)==1),2));
cnst_prep_cads_gmsh = polyfit (wdofs(fgeou),wprep(fgeou),0),% geo_unrolled
cnst_prep_cads_bash = polyfit (wdofs(fgeos),wprep(fgeos),0),% geo
cnst_chck_cads_geos = polyfit (wdofs(fcads),wchck(fcads),0),
%
loglog (dofs(fmesh), prep(fmesh),'<k','markersize',sms,'linewidth',mlw);
loglog (dofs(fgeou), prep(fgeou),'<c','markersize',ms,'linewidth',mlw);
loglog (dofs(fgeos), prep(fgeos),'<b','markersize',ms,'linewidth',mlw);
%
loglog (dofs(fothr), chck(fothr),'+k','markersize',sms,'linewidth',mlw);
loglog (dofs(fcads), chck(fcads),'+b','markersize',sms,'linewidth',mlw);
loglog (dofs(fcgns), chck(fcgns),'+g','markersize',sms,'linewidth',mlw);
loglog (dofs(fmshb), chck(fmshb),'+y','markersize',sms,'linewidth',mlw);
%
loglog (dofs, mesh,'^b','markersize',sms+1,'linewidth',mlw);
%
loglog (dofs(fread), read(fread),'ok','markersize',sms,'linewidth',mlw);
loglog (dofs(fcgns), read(fcgns),'og','markersize',sms,'linewidth',mlw);
%loglog (dofs(fmshb), read(fmshb),'oy','markersize',sms,'linewidth',mlw);
%
loglog (dofs, solv,'*m','markersize',sms-1,'linewidth',mlw);
loglog (dofs(fmesh), simt(fmesh),'xm','markersize',sms,'linewidth',hlw);
loglog (dofs(fcgns), simt(fcgns),'xg','markersize',sms,'linewidth',lw);
loglog (dofs(fgeou), simt(fgeou),'xc','markersize',sms,'linewidth',mlw);
loglog (dofs(fgeos), simt(fgeos),'xr','markersize',ms,'linewidth',lw);
%
loglog (dofs_span, polyval(poly_prep_mesh,dofs_span),'--k','linewidth',mlw);
loglog (dofs_span, polyval(cnst_prep_cads_gmsh,dofs_span),'--c','linewidth',hlw);
loglog (dofs_span, polyval(cnst_prep_cads_bash,dofs_span),'--b','linewidth',flw);
%
loglog (dofs_span, polyval(poly_chck     ,dofs_span),':k','linewidth',mlw);
loglog (dofs_span, polyval(poly_chck_msh4,dofs_span),'-y','linewidth',lw);
loglog (dofs_span, polyval(poly_chck_cgns,dofs_span),':g','linewidth',mlw);
loglog (dofs_span, polyval(cnst_chck_cads_geos,dofs_span),':b','linewidth',hlw);
%
loglog (dofs_span, polyval(poly_mesh_cads,dofs_span),'-.b','linewidth',mlw);
%
%TODO plot post times (Gmsh vs. CGNS)?
%
%loglog (dofs_span, polyval(poly_read     ,dofs_span),'-k','linewidth',hlw);
%loglog (dofs_span, polyval(poly_read_mshb,dofs_span),'-k','linewidth',hlw);
loglog (dofs_span, polyval(poly_read_cgns,dofs_span),'-g','linewidth',mlw);
%
femera_cgns_data_speedup(1) = (poly_chck_msh4(1)*2)  / poly_read_cgns(1);
femera_cgns_data_speedup(2) = (poly_chck_msh4(1)+poly_chck(1))/poly_read_cgns(1);
femera_cgns_data_speedup,
%
femera_cads_mesh_speedup = poly_prep_mesh(1) / poly_mesh_cads(1),
%
%run_time = (max(csv(:,7)) - min(csv(:,6)))*ns,
%tot_time = run_time + sum(mesh_time),
%avg_run_time = run_time / sim_n,
%avg_tot_time = tot_time / sim_n,
%
speed = 1e9 - (dofs_span>10e3).* 90e6.*(log(dofs_span)-log(10e3));
speed (speed < 500e6) = 500e6;
poly = [1/3,1.5];
iters = exp (polyval (poly, log (dofs_span)));
secs = iters .* dofs_span ./ speed;% avg time to solve 1 sim (using 40 cores)
loglog (dofs_span,secs,'-m','linewidth',lw);% estimated solve (only) time
%
%
figure (2); hold on;
paper=[0.25,0.25, 6,4];
set(gcf,'paperposition',paper);
figname=['sm-md-simtime-',solvshort],
print([figdir,figname,'.eps'],'-depsc2','-FHelvetica');
print([figdir,figname,'.pdf'],'-depsc2','-FHelvetica');
%
