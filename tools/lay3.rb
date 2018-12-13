#!/usr/bin/ruby -Ku
# 
# 	ラック配置→トポロジ生成
# 	Layout racks on a floor and then create a topology of nodes
# 	Ikki Fujiwara <ikki@nii.ac.jp>
# 

RACK_WIDTH     =  60 # Rack width [cm]
RACK_DEPTH     = 210 # Rack depth including aisle [cm]
OVERHEAD_OUTER = 400 # Inter-rack cabling overhead [cm]
OVERHEAD_INNER = 200 # Intra-rack cabling overhead [cm]
RAND_TRIAL     =  10 # Number of trial of random matching

$help_message = <<-EOM
Usage: #{$0} [Options]... Geometry Topologies...

Options:
  -u            create unidirectional (directed) arcs instead of edges
  -f            overwrite existing log file
  -c            create per-rack (clustered) edgelist file
  -q            be quiet
  -v<LEVEL>     be verbose
  --floor=<DEPTH>x<WIDTH>
                number of racks on a floor [default: square-numbered (e.g. 4x4)]
  --seed=<SEED> random seed

Geometry:
  <N>r<Z>       put N nodes in N/Z racks each containing Z nodes
  <R>x<Z>       put R*Z nodes in R racks each containing Z nodes

Topologies:
  <FILE>        read per-node edgelist
  hc[-<D>]      hypercube [default: auto dimensions]
  mesh[-<A>]    mesh [default: 3-D geometry-based]
  torus[-<A>]   torus [default: 3-D geometry-based]
  ftorus[-<A>]  folded torus [default: 3-D geometry-based]
  ring          ring
  rr-<D>        ring plus random shortcuts
  r-<D>         random
  hx            hyperx, geometry-based
  df[-<a>-<h>]  dragonfly (see Kim's "Dragonfly" paper)
  sky[-<di>-<do>] skywalk (see Ikki's "Skywalk" paper)
  cut-<P>       link failure
where
  <A>           #nodes for each dimension (e.g. '5-4-3')
  <D>           degree per node (twice for unidirectional)
  <P>           percentage or proportion (e.g. '25%' or '0.25')
  <FILE>        edgelist filename (e.g. 'mygraph.edges')
See 'wire' method for complete list of available topologies.

Files created:
  *.coord       coordinates of nodes and racks on the floor
  *.edges       per-node edgelist with cable length
  *.clu.edges   per-rack edgelist with number of cables (given -c option)

Standard output:
See 'disp_cost' method for the meanings of the values displayed.
EOM

require "optparse"
require "pp"

def main
	$opts = ARGV.getopts("ufcqv:", "floor:", "layout:", "seed:", "help")
	$opts["floor"] ||= $opts["layout"] # for backward compatibility
	$opts["v"] = $opts["v"].to_i
	seed = $opts["seed"].to_i
	if $opts["help"] || ARGV.length < 2
		puts $help_message
		exit(1)
	end
	g = MyNetwork.new($opts["u"])
	tnames = ARGV[1..-1].collect{|arg| g.name_topology(arg) }
	myname = [ARGV[0]].concat(tnames).join("+")
	myname += "[#{seed}]" if $opts["seed"]
	srand(seed)
	
	ext_edges = ($opts["u"]) ? ".arcs" : ".edges"
	fnames = Hash.new
	fnames["floor"] = ARGV[0] + ".coord"
	fnames["edges"] = myname + ext_edges
	fnames["clu.edges"] = myname + ".clu" + ext_edges
	if File.exist?(fnames["edges"]) && !$opts["f"]
		STDERR.puts "Output file '#{fnames['edges']}' exists. Use -f to overwrite" if !$opts["q"]
		exit(0)
	end
	g.make_geometry(ARGV[0], $opts["floor"])
	ARGV[1..-1].each do |arg|
		g.make_topology(arg)
	end
	g.save_geometry(fnames["floor"])
	g.save_edgelist(fnames["edges"])
	g.save_clustered_edgelist(fnames["clu.edges"]) if $opts["c"]
	g.disp_cost(myname)
end

# コスト計算・表示
module CostModel
	def disp_cost(myname)
		cable_count = 0
		cable_amount = 0
		cable_cost = 0
		switch_cost = 0
		switch_power = 0
		edges.each do |n1, n2|
			cable_count += 1
			cable_amount += cable_between(n1, n2)
			cable_cost += cost_of_cable(cable_between(n1, n2))
# 			STDERR.puts "  #{n1}--#{n2}\t#{cable_between(n1, n2)}cm\t$#{cost_of_cable(cable_between(n1, n2))}" if $opts["v"] >= 4
		end
		@nodes.each do |n|
# 			STDERR.puts "  #{n}\tdeg=#{degree(n)}\t$#{cost_of_switch(degree(n))}" if $opts["v"] >= 4
			switch_cost += cost_of_switch(degree(n))
			switch_power += power_of_switch(degree(n))
		end
		total_cost = cable_cost + switch_cost
		STDOUT.puts format("%s\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%d\t%d\t%0.2f\t%0.2f", myname, total_cost, cable_cost, switch_cost, 0.0, cable_amount, cable_count, cable_amount.to_f / cable_count, switch_power)
	end
	# from Besta's "Slim Fly" paper (2014)
	#   Electric cable cost:
	#     (0.4079 * x [m] + 0.5771) [$/Gbps] * 40 [Gbps]
	#   Optical cable cost:
	#     (0.0919 * x [m] + 7.2745) [$/Gbps] * 40 [Gbps]
	#   Switch cost:
	#     (350.4 * k [ports] − 892.3) [$]
	#   Switch power:
	#     (4 * 0.7 * k [ports]) [W]
	def cost_of_cable(length)
		electric = (length.to_f / 100 * 0.4079 + 0.5771) * 40
		optical  = (length.to_f / 100 * 0.0919 + 7.2745) * 40
		if length <= OVERHEAD_INNER
			dollar = electric
		elsif electric <= optical
			dollar = electric
		else
			dollar = optical
		end
		(dollar * 100).round.to_f / 100
	end
	def cost_of_switch(nports)
		dollar = nports * 350.4 - 892.3
		dollar
	end
	def power_of_switch(nports)
		watt = nports * 4 * 0.7
		watt
	end
# 	# from Mudigonda's "Taming the flying cable monster" paper (2011)
# 	#   Switch cost:
# 	#     $500/port
# 	#   Custom cable cost:
# 	#     Single-channel (server <-> switch):
# 	#       copper $6/m + $40  (<=5m)
# 	#     Quad-channel (switch <-> switch):
# 	#       copper $16/m + $40 (<=5m)
# 	#       fiber  $5/m + $376 (>5m)
# 	#     plus 25% as a manufacturing cost
# 	#   Stock cable cost:
# 	#     SFP copper (server <-> switch):
# 	#       1m $45; 2m $52; 3m $66; 5m $74; 10m $101; 12m $117
# 	#     QSFP copper (switch <-> switch, <=5m):
# 	#       1m $55; 2m $74; 3m $87; 5m  $116
# 	#     QSFP+ fiber (switch <-> switch, >5m):
# 	#       10m $418; 15m $448; 20m $465; 30m $508; 50m $618; 100m $883
# 	#   Deployment cost:
# 	#     intra-rack $2.50
# 	#     inter-rack $6.25
# 	def cost_of_cable(length)
# 		if length <= OVERHEAD_INNER
# 			dollar = (length.to_f / 100 * 16 + 40) * 1.25 + 2.50
# 		elsif length <= 500
# 			dollar = (length.to_f / 100 * 16 + 40) * 1.25 + 6.25
# 		else
# 			dollar = (length.to_f / 100 * 5 + 376) * 1.25 + 6.25
# 		end
# 		(dollar * 100).round.to_f / 100
# 	end
# 	def cost_of_switch(nports)
# 		dollar = nports * 500
# 		dollar
# 	end
end

# 文字列
class String
	# 適当に数値化
	def to_num(unit = "")
		x = self.to_f
		u = Regexp.quote(unit)
		if suffix = match(/^\s*[+-]?[\d.]+[ \t]*([%mcdk])#{u}/)
			case suffix[1]
			when "%" then x /= 100
			when "m" then x /= 1000
			when "c" then x /= 100
			when "d" then x /= 10
			when "k" then x *= 1000
			end
		end
		(x.to_i == x) ? x.to_i : x
	end
end

# 配列
class Array
	# 要素を適当に数値化
	def to_nums
		self.collect do |x|
			if x.class == String
				x.to_num
			else
				(x.to_i == x.to_f) ? x.to_i : x.to_f
			end
		end
	end
	# 和
	def sum
		self.inject(0){|x, y| x + y }
	end
	# 積
	def mul
		self.inject(1){|x, y| x * y }
	end
	# 平均
	def avg
		self.sum.to_f / self.length
	end
	# ヒストグラム
	def histogram
		h = Hash.new(0)
		self.sort.each do |x|
			h[x] += 1
		end
		h
	end
	# 複数の配列の同じ位置にある要素同士を組み合わせてイテレート
	# 	[[1,2,3,4],[5,6,7],[8,9]].each_pair => [1,5,8],[2,6,9],[3,7,nil],[4,nil,nil]
	def each_pair
		self.max_by{|x| x.length }.each_index do |i|
			yield(self.collect{|x| x[i] })
		end
	end
	# 複数の配列からそれぞれ1個の要素を選んだ配列の配列を返す
	# 	[[1,2],[3,4,5]].self_product => [[1,3],[1,4],[1,5],[2,3],[2,4],[2,5]]
	def self_product
		if self.empty?
			[[]]
		elsif self[0].is_a?(Array)
			self[0].product(*self[1..-1])
		else
			raise TypeError
		end
	end
	# ランダムマッチングを生成
	# 	ブロックにペア案が渡される。ブロックの値が真だったペアをすべて含む配列を返す
	def random_matching
		pairs = Array.new
		pool = self.shuffle
		until pool.empty?
			x = pool.shift
			pool.each_with_index do |y, i|
				if yield(x, y)
					pool.delete_at(i)
					pairs.push([x, y])
					break
				end
			end
		end
		pairs
	end
end

# 重複グラフ
# 	辺の本数を辺の width で表現する。width == 0 の辺は存在しない。次数は width の合計。
class MyMultigraph
	include Enumerable
	attr_reader :edge_data, :vertex_data, :edge_count
	def initialize(directed)
		@directed = directed
		@edge_data = Hash.new{|hash, key| hash[key] = Hash.new(0) }
		@edge_count = 0
		@vertex_data = Hash.new{|hash, key| hash[key] = Hash.new(0) }
	end
	def each
		@edge_data.each do |i, h|
			h.each do |j, w|
				yield(i, j, w)
			end
		end
	end
	def edges
		sort
	end
	def vertices
		@vertex_data.keys.sort
	end
	def vertex_count
		@vertex_data.length
	end
	def include_edge?(i, j)
		@edge_data.has_key?(i) && @edge_data[i].has_key?(j)
	end
	private :include_edge?
	def has_edge?(i, j)
		i, j = j, i if !(@directed || i <= j)
		include_edge?(i, j)
	end
	def has_vertex?(k)
		@vertex_data.has_key?(k)
	end
	def width(i, j)
		i, j = j, i if !(@directed || i <= j)
		(include_edge?(i, j)) ? @edge_data[i][j] : nil
	end
	def degree(k)
		(has_vertex?(k)) ? @vertex_data[k][:outdegree] + @vertex_data[k][:indegree] : nil
	end
	def outdegree(k)
		(has_vertex?(k)) ? @vertex_data[k][:outdegree] : nil
	end
	def indegree(k)
		(has_vertex?(k)) ? @vertex_data[k][:indegree] : nil
	end
	def degrees
		a = Array.new
		@vertex_data.each_key do |k|
			a[k] = degree(k)
		end
		a
	end
	def add_edge?(i, j, w = 1)
		# w = 0: allowed but does nothing
		raise ArgumentError "Negative width is not allowed" unless w >= 0
		i, j = j, i if !(@directed || i <= j)
		if w > 0
			@edge_data[i][j] += w
			@edge_count += w
			@vertex_data[i][:outdegree] += w
			@vertex_data[j][:indegree] += w
			self
		else
			nil
		end
	end
	def add_edge(i, j, w = 1)
		add_edge?(i, j, w)
		self
	end
	def delete_edge?(i, j, w = 0)
		# w = 0: delete the whole edge
		raise ArgumentError "Negative width is not allowed" unless w >= 0
		i, j = j, i if !(@directed || i <= j)
		if include_edge?(i, j)
			if w > 0 && w < @edge_data[i][j]
				@edge_data[i][j] -= w
			else
				w = @edge_data[i][j]
				@edge_data[i].delete(j)
				@edge_data.delete(i) if @edge_data[i].empty?
			end
			@edge_count -= w
			@vertex_data[i][:outdegree] -= w
			@vertex_data[j][:indegree] -= w
			self
		else
			nil
		end
	end
	def delete_edge(i, j, w = 0)
		delete_edge?(i, j, w)
		self
	end
	def scan_edge(str)
		if s = str.match(/^\s*(\d+)\s+(\d+)\s+(\d+)/)
			add_edge?(s[1].to_i, s[2].to_i, s[3].to_i)
		else
			raise "Edge string must begin with three numbers separated by whitespaces: '#{str}'"
		end
	end
	def format_edge(i, j, w)
		format("%d %d %d", i, j, w)
	end
	def load_edgelist(filename)
		open(filename, "r") do |f|
			f.each_line do |line|
				scan_edge(line)
			end
		end
	end
	def save_edgelist(filename)
		open(filename, "w") do |f|
			edges.each do |i, j, w|
				f.puts(format_edge(i, j, w))
			end
		end
	end
end

# 単純グラフ
class MyGraph < MyMultigraph
	def add_edge?(i, j, w = nil)
		if i != j && !has_edge?(i, j)
			super(i, j)
		else
			nil
		end
	end
	def scan_edge(str)
		if s = str.match(/^\s*(\d+)\s+(\d+)/)
			add_edge?(s[1].to_i, s[2].to_i)
		else
			raise "Edge string must begin with two numbers separated by whitespaces: '#{str}'"
		end
	end
	def format_edge(i, j, w = nil)
		format("%d %d", i, j)
	end
end

# 座標つき単純グラフ
class MyMap < MyGraph
	def initialize(directed)
		super(directed)
		@nodes = Array.new # 全ノードリスト
		@racks = Array.new # 全ラックリスト
		@racksize = 0 # 1ラックあたり最大ノード数
		@xs = Array.new # X 座標リスト (ラックの奥行方向)
		@ys = Array.new # Y 座標リスト (ラックの幅方向)
		@nodes_in = Hash.new # @nodes_in[ラック番号] = ノードリスト
		@rack_of  = Hash.new # @rack_of[ノード番号] = ラック番号
		@coord_of = Hash.new # @coord_of[ラック番号] = 座標
		@rack_at  = Hash.new # @rack_at[座標] = ラック番号
		@cycle_in = Hash.new # @cycle_in[ラック番号] = ノード循環列挙子
		@arrows = [ # デバッグ表示用
			["   ", "---", "   ", "-->"],
			["   ", "-->", "<--", "==="],
			["   ", "-/-", "   ", "-/>"],
			["   ", "-/>", "</-", "=/="],
			["---", "-->"]
		]
	end
	# フロアプランを生成
	def make_geometry(arg, floor)
		case arg
		when /(\d+)x(\d+)/
			nracks = $1.to_i # ラック数
			@racksize = $2.to_i # ラックサイズ
			nnodes = nracks * @racksize # ノード数
		when /(\d+)\D+(\d+)/
			nnodes = $1.to_i # ノード数
			@racksize = $2.to_i # ラックサイズ
			nracks = (nnodes.to_f / @racksize).ceil # ラック数
		else
			raise "Geometry must include #nodes and #racks"
		end
		case floor
		when /(\d+)\D+(\d+)/
			nx = $1.to_i # 奥行方向のラック数
			ny = $2.to_i # 幅方向のラック数
			raise "Cannot put #{nracks} racks on a #{nx}x#{ny} floor" unless nracks <= nx * ny
		when "room"
			ra = Math.sqrt(nracks * RACK_WIDTH * RACK_DEPTH) # 部屋の辺長
			nx = [(ra / RACK_DEPTH).floor, 1].max # 奥行方向のラック数
			ny = (nracks.to_f / nx).ceil # 幅方向のラック数
		else
			nx = Math.sqrt(nracks).ceil  # 奥行方向のラック数
			ny = (nracks.to_f / nx).ceil # 幅方向のラック数
		end
		STDERR.puts "#{nnodes} nodes in #{nracks} racks on a #{nx}x#{ny} floor" if $opts["v"] >= 1
		@nodes = (0...nnodes).to_a
		@racks = (0...nracks).to_a
		@nodes.each do |n|
			r = ((n * nracks).to_f / nnodes).floor
			@nodes_in[r] ||= Array.new
			@nodes_in[r].push(n)
			@rack_of[n] = r
		end
		@racks.each do |r|
			x = (r / ny) * RACK_DEPTH
			y = (r % ny) * RACK_WIDTH
			@coord_of[r] = [x, y]
			@rack_at[[x, y]] = r
			@cycle_in[r] = @nodes_in[r].cycle
			@xs.push(x)
			@ys.push(y)
		end
		@xs = @xs.uniq.sort
		@ys = @ys.uniq.sort
	end
	# ノード列挙子をリセット
	def reset_grouping(groupsize)
		STDERR.puts("#{groupsize} nodes per group") if $opts["v"] >= 1
		@cycle_in = Hash.new # @cycle_in[グループ番号] = ノード循環列挙子
		nnodes = @nodes.length # ノード数
		ngroups = (nnodes.to_f / groupsize).ceil # グループ数
		nodes_in = @nodes.group_by{|n| ((n * ngroups).to_f / nnodes).floor }
		(0...ngroups).each do |r|
			@cycle_in[r] = nodes_in[r].cycle
		end
		nodes_in
	end
	# フロアプランを読み込む
	def load_geometry(filename)
		data = Hash.new{|hash, key| hash[key] = Set.new }
		open(filename, "r") do |f|
			f.each_line do |line|
				# *.coord ファイルは X, Y が逆
				n, r, y, x = line.split # split by whitespaces
				next unless n && r && y && x
				n = n.to_i # ノード番号
				r = r.to_i # ラック番号
				x = x.to_i # X 座標
				y = y.to_i # Y 座標
				data[r].add(n)
				@rack_of[n] = r
				@coord_of[r] = [x, y]
				@rack_at[[x, y]] = r
				@xs.push(x)
				@ys.push(y)
			end
		end
		data.each do |r, nset|
			@nodes_in[r] = nset.sort
			@nodes.concat(@nodes_in[r])
			@racksize = nset.length if @racksize < nset.length
		end
		@nodes.sort!
		@racks = data.keys.sort
		@xs = @xs.uniq.sort
		@ys = @ys.uniq.sort
	end
	# フロアプランを書き出す
	def save_geometry(filename)
		open(filename, "w") do |f|
			@rack_of.each do |node, rack|
				x, y = @coord_of[rack]
				# *.coord ファイルは X, Y が逆
				f.printf("%d %d %d %d\n", node, rack, y, x)
			end
		end
	end
	# 配線長を重みとするエッジリストを書き出す
	def save_edgelist(filename)
		open(filename, "w") do |f|
			edges.each do |n1, n2|
				f.printf("%d %d %d\n", n1, n2, cable_between(n1, n2))
			end
		end
	end
	# 配線数を重みとするラック単位のエッジリストを書き出す
	def save_clustered_edgelist(filename)
		grack = MyMultigraph.new(false)
		each do |n1, n2|
			r1 = @rack_of[n1]
			r2 = @rack_of[n2]
			grack.add_edge(r1, r2, 1) if r1 != r2
		end
		grack.save_edgelist(filename)
		return
		# ログファイルも作る
		ntube = 0
		nlink = 0
		grack.each do |i, j, w|
			raise "oops" unless i < j
			ntube += 1
			nlink += w
		end
		logfn = filename.sub(/\.\w+$/){ ".log" }
		open(logfn, "w") do |f|
			f.printf("#node      = %d\n", @nodes.length)
			f.printf("#edge      = %d\n", edge_count)
			f.printf("#rack      = %d\n", @racks.length)
			f.printf("#tube      = %d\n", ntube)
			f.printf("#link      = %d\n", nlink)
			f.printf("membership = %s\n", @nodes.collect{|n| @rack_of[n] }.join(","))
		end
	end
	# 配線長
	def cable_between(n1, n2)
		cable_of(dist_manhattan(@coord_of[@rack_of[n1]], @coord_of[@rack_of[n2]])).round
	end
	# 距離 → 配線長
	def cable_of(dist)
		(dist == 0) ? OVERHEAD_INNER : OVERHEAD_OUTER + dist
	end
	# ユークリッド距離
	def dist_euclidean(p, q)
		Math.sqrt((p[0] - q[0]) ** 2 + (p[1] - q[1]) ** 2)
	end
	# マンハッタン距離
	def dist_manhattan(p, q)
		(p[0] - q[0]).abs + (p[1] - q[1]).abs
	end
	alias :dist_between :dist_manhattan
	# ノード次数
	def degree(n)
		if !@directed
			super(n)
		else
			d = super(n) / 2.0
			(d.to_i == d) ? d.to_i : d
		end
	end
	# リンクを追加
	# 	unidir = true:単方向  false:双方向
	# 	返り値 = 実際に追加されたリンク数
	def add_link(n1, n2, unidir = false)
		return nil if !n1 || !n2 || n1 == n2
		raise "Use -u to use unidirectional links" if unidir && !@directed
		if unidir || !@directed
			x1 = (add_edge?(n1, n2)) ? 1 : 0
			arrow = @arrows[0][x1 + ((unidir) ? 2 : 0)]
			count = x1
		else
			x1 = (add_edge?(n1, n2)) ? 1 : 0
			x2 = (add_edge?(n2, n1)) ? 1 : 0
			arrow = @arrows[1][x1 + x2 * 2]
			count = x1 + x2
		end
		STDERR.print format("  %3d %s %-3d\n", n1, arrow, n2) if $opts["v"] >= 4
		count
	end
	def add_links(a, unidir = false)
		a.inject(0){|x, (n1, n2)| x + add_link(n1, n2, unidir) }
	end
	# リンクを削除
	# 	unidir = true:単方向  false:双方向
	# 	返り値 = 実際に削除されたリンク数
	def delete_link(n1, n2, unidir = false)
		return nil if !n1 || !n2 || n1 == n2
		raise "Use -u to use unidirectional links" unless @directed || !unidir
		if unidir || !@directed
			x1 = (delete_edge?(n1, n2)) ? 1 : 0
			arrow = @arrows[2][x1 + ((unidir) ? 2 : 0)]
			count = x1
		else
			x1 = (delete_edge?(n1, n2)) ? 1 : 0
			x2 = (delete_edge?(n2, n1)) ? 1 : 0
			arrow = @arrows[3][x1 + x2 * 2]
			count = x1 + x2
		end
		STDERR.print format("  %3d %s %-3d\n", n1, arrow, n2) if $opts["v"] >= 4
		count
	end
	def delete_links(a, unidir = false)
		a.inject(0){|x, (n1, n2)| x + delete_link(n1, n2, unidir) }
	end
	# ラック間リンクを追加
	# 	unidir = true:単方向  false:双方向
	# 	返り値 = 実際に追加されたリンク数
	def add_quasi_link(r1, r2, unidir = false)
		arrow = @arrows[4][(unidir) ? 1 : 0]
		STDERR.print format("  [%3d]%s[%3d]", r1, arrow, r2) if $opts["v"] >= 4
		add_link(@cycle_in[r1].next, @cycle_in[r2].next, unidir)
	end
	def add_quasi_links(a, unidir = false)
		a.inject(0){|x, (r1, r2)| x + add_quasi_link(r1, r2, unidir) }
	end
end

# ネットワーク
class MyNetwork < MyMap
	include CostModel
	# トポロジ名を返す
	def name_topology(arg)
		if File.exist?(arg)
			return File.basename(arg, ".*")
		end
		desc, *fnames = arg.split("@")
		fnames.collect!{|fn| File.basename(fn, ".*") }
		[desc].concat(fnames).join("@")
	end
	# リンクを張る
	def make_topology(arg)
		if File.exist?(arg)
			return load_edgelist(arg)
		end
		desc, *fnames = arg.split("@")
		topo, *params = desc.split("-")
		geometry = [@xs.length, @ys.length, @racksize]
		case topo
		when "hc" # Hypercube
			params[0] ||= (Math.log(@nodes.length) / Math.log(2)).ceil
			cube([2] * params[0].to_i){|nodes| link_line(nodes) }
		when "mesh" # Mesh
			params = geometry if params.empty?
			cube(params.to_nums){|nodes| link_line(nodes) }
		when "torus" # Torus
			params = geometry if params.empty?
			cube(params.to_nums){|nodes| link_ring(nodes) }
		when "ftorus" # Folded Torus
			params = geometry if params.empty?
			cube(params.to_nums){|nodes| link_folded_ring(nodes) }
		when "ring" # Ring
			link_ring(@nodes)
		when "rr" # Random Ring
			link_ring(@nodes)
			link_random(@nodes, params[0].to_i - 2)
		when "r" # Random
			link_random(@nodes, params[0].to_i, false)
		when "ur" # Unidirectional Random
			link_random(@nodes, params[0].to_i * 2, true)
		when "db" # De Bruijn
			link_debruijn(@nodes, params[0].to_i, params[1].to_i)
		when "hx" # HyperX: Z/Y/X方向ノード間完全結合
			nodes_along_z{|nodes| link_full(nodes) }
			nodes_along_y{|nodes| link_full(nodes) }
			nodes_along_x{|nodes| link_full(nodes) }
		when "df" # Dragonfly: グループ内完全結合＋全グループ間完全結合
			if params[0]
				params[1] ||= params[0].to_i / 2
				g = reset_grouping(params[0].to_i)
				g.values.each{|nodes| link_full(nodes) }
				link_quasi_overfull(g.keys, params[0].to_i, params[1].to_i)
			else # Dragonfly: Z方向ノード間完全結合＋全ラック間完全結合
				nodes_along_z{|nodes| link_full(nodes) }
				link_quasi_full(@racks)
			end
		when "sky" # Skywalk: Z方向ノード間ランダム結合＋Y/X方向ラック間ランダム結合
			di_max = @racksize - 1
			ds_max = (((@xs.length - 1) + (@ys.length - 1)) / @racksize.to_f).ceil
			params[0] ||= di_max
			params[1] ||= ds_max
			if params[1].to_i <= ds_max
				params[2] = 0
			else
				params[2] = params[1].to_i - ds_max
				params[1] = ds_max
			end
			if params[0].to_i < di_max
				nodes_along_z{|nodes| link_random(nodes, params[0].to_i) }
			else
				nodes_along_z{|nodes| link_full(nodes) }
			end
			if params[1].to_i < ds_max
				link_quasi_random(@racks, @racksize, params[1].to_i) do |r1, r2|
					@coord_of[r1][0] == @coord_of[r2][0] || @coord_of[r1][1] == @coord_of[r2][1]
				end
			else
				racks_along_y{|racks| link_quasi_full(racks) }
				racks_along_x{|racks| link_quasi_full(racks) }
			end
			if params[2] > 0
				link_quasi_random(@racks, @racksize, params[2].to_i) do |r1, r2|
					@coord_of[r1][0] != @coord_of[r2][0] && @coord_of[r1][1] != @coord_of[r2][1]
				end
			end
		when "cut" # Link Failure
			unlink_random((edge_count * params[0].to_num).ceil, false)
		when "ucut" # Unidirectional Link Failure
			unlink_random((edge_count * params[0].to_num).ceil, true)
		else
			raise "Topology '#{topo}' is not defined"
		end
		STDERR.puts " degrees: #{degrees.histogram}" if $opts["v"] >= 3
	end
	# ノードを超直方体の全方向に串刺し (フロアとは無関係)
	# 	sizes = 各方向のノード数
	def cube(sizes)
		# 例: sizes = [4,3,2]
		bases = Array.new
		sizes.reverse.inject(1) do |k, l|
			bases.unshift(Array.new(l){|j| j * k })
			k * l
		end
		# 例: bases = [[0,6,12,18],[0,2,4],[0,1]]
		bases.each_index do |i|
			b = bases.dup
			h = b.delete_at(i)
			# 例: i = 1; h = [0,2,4]; b = [[0,6,12,18],[0,1]]
			b.self_product.collect{|a| a.sum }.each do |k|
				# 例: k = 0,1,6,7,12,13,18,19
				yield(h.collect{|j| @nodes[j + k] }.compact)
			end
		end
	end
	# ノードをフロア上の X 方向に串刺し
	def nodes_along_x
		racks_along_x do |rs|
			@racksize.times do |z|
				yield(rs.collect{|r| @nodes_in[r][z] }.compact)
			end
		end
	end
	# ノードをフロア上の Y 方向に串刺し
	def nodes_along_y
		racks_along_y do |rs|
			@racksize.times do |z|
				yield(rs.collect{|r| @nodes_in[r][z] }.compact)
			end
		end
	end
	# ノードをフロア上の Z 方向に串刺し
	def nodes_along_z
		@racks.each do |r|
			yield(@nodes_in[r])
		end
	end
	# ラックをフロア上の X 方向に串刺し
	def racks_along_x
		@ys.each do |y| 
			yield(@xs.collect{|x| @rack_at[[x, y]] }.compact)
		end
	end
	# ラックをフロア上の Y 方向に串刺し
	def racks_along_y
		@xs.each do |x|
			yield(@ys.collect{|y| @rack_at[[x, y]] }.compact)
		end
	end
	# 線状リンク
	def link_line(nodes)
		STDERR.puts "link_line(#{nodes})" if $opts["v"] >= 2
		nodes.each_cons(2) do |n1, n2|
			add_link(n1, n2)
		end
	end
	# 環状リンク
	def link_ring(nodes)
		STDERR.puts "link_ring(#{nodes})" if $opts["v"] >= 2
		nodes.each_cons(2) do |n1, n2|
			add_link(n1, n2)
		end
		add_link(nodes[0], nodes[-1])
	end
	# 折りたたみ環状リンク
	def link_folded_ring(nodes)
		STDERR.puts "link_folded_ring(#{nodes})" if $opts["v"] >= 2
		add_link(nodes[0], nodes[1]) 
		nodes.each_cons(3) do |n1, n2, n3|
			add_link(n1, n3)
		end
		add_link(nodes[-2], nodes[-1])
	end
	# 完全リンク
	def link_full(nodes)
		STDERR.puts "link_full(#{nodes})" if $opts["v"] >= 2
		nodes.combination(2) do |n1, n2|
			if !block_given? || yield(n1, n2)
				add_link(n1, n2)
			end
		end
	end
	# ランダムリンク (次数指定、単方向も可)
	def link_random(nodes, deg, unidir = false)
		STDERR.puts "link_random(#{nodes}, #{deg})" if $opts["v"] >= 2
		raise "Too small degree (#{deg})" unless deg >= 0
		STDERR.puts "Warning: Too large degree (#{deg})" unless deg < nodes.length if !$opts["q"]
		best = nil
		RAND_TRIAL.times do |t|
			plan = MyGraph.new(unidir)
			deg.times do |d|
				nodes.random_matching do |n1, n2|
					if !block_given? || yield(n1, n2)
						!self.has_edge?(n1, n2) && plan.add_edge?(n1, n2)
					end
				end
			end
			STDERR.print " #{plan.edge_count}" if $opts["v"] == 3
			best = plan if !best || best.edge_count < plan.edge_count
			break if best.edge_count == nodes.length / 2 * deg
		end
		count = add_links(best, unidir)
		STDERR.puts " => #{count} links added" if $opts["v"] >= 3
	end
	# ラック間完全リンク
	def link_quasi_full(racks)
		STDERR.puts "link_quasi_full(#{racks})" if $opts["v"] >= 2
		racks.combination(2) do |r1, r2|
			if !block_given? || yield(r1, r2)
				add_quasi_link(r1, r2)
			end
		end
	end
	# ラック間ランダムリンク (ノード次数指定)
	def link_quasi_random(racks, racksize, deg)
		STDERR.puts "link_quasi_random(#{racks}, #{racksize}, #{deg})" if $opts["v"] >= 2
		raise "Too small degree (#{deg})" unless deg >= 0
		best = nil
		RAND_TRIAL.times do |t|
			plan = MyGraph.new(false)
			(deg * racksize).times do |d|
				racks.random_matching do |r1, r2|
					if !block_given? || yield(r1, r2)
						plan.add_edge?(r1, r2)
					end
				end
			end
			STDERR.print " #{plan.edge_count}" if $opts["v"] == 3
			best = plan if !best || best.edge_count < plan.edge_count
			break if best.edge_count == racks.length / 2 * deg
		end
		count = add_quasi_links(best)
		STDERR.puts " => #{count} links added" if $opts["v"] >= 3
	end
	# ラック間重複リンク (ノード次数指定)
	def link_quasi_overfull(racks, racksize, deg)
		STDERR.puts "link_quasi_overfull(#{racks}, #{racksize}, #{deg})" if $opts["v"] >= 2
		gdeg = racksize * deg
		raise "Quasi-full links among #{racks.length} racks requires degree >= #{(racks.length - 1) / racksize.to_f}" unless racks.length - 1 <= racksize * deg
		gdegs = Hash.new(0)
		(gdeg.to_f / (racks.length - 1)).ceil.times do |t|
			link_quasi_full(racks) do |r1, r2|
				if gdegs[r1] < gdeg && gdegs[r2] < gdeg
					gdegs[r1] += 1
					gdegs[r2] += 1
					true
				else
					false
				end
			end
		end
	end
	# ドブロイリンク
	def link_debruijn(nodes, deg, n)
		STDERR.puts "link_debruijn(#{nodes}, #{deg}, #{n})" if $opts["v"] >= 2
		k = deg ** n
		raise "De Bruijn graph must have exactly #{deg}^#{n} = #{k} nodes" unless nodes.length == k
		nodes.each_index do |i|
			deg.times do |d|
				j = (i * deg + d) % k
				STDERR.print format("  %#{n}s --> %#{n}s", i.to_s(deg), j.to_s(deg)) if $opts["v"] >= 3
				add_link(nodes[i], nodes[j], true)
			end
		end
	end
	# ランダム切断 (本数指定、単方向も可)
	def unlink_random(n, unidir = false)
		STDERR.puts "unlink_random(#{n})" if $opts["v"] >= 2
		delete_links(edges.sample(n), unidir)
	end
end

main if $0 == __FILE__
