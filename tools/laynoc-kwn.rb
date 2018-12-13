#!/usr/bin/ruby -Ku
# -*- coding: utf-8 -*-
# 
# 	3次元積層チップのトポロジ生成
# 	Ryuta Kawano <kawano@am.ics.keio.ac.jp>
# 

CORE_SIZE = 100 # コアサイズ (正方形の辺長、単位は任意)
# H-treeの配線長を求める場合、コアサイズは log4(コア数) で割り切れる値にすること。
RAND_TRIAL = 10 # ランダムマッチング試行回数
RAND_TRIAL_HOST = 1000000 # ランダムマッチング試行回数(ホストリンク用)

$help_message = <<-EOM
Usage:
  #{$0} [Options] Dimension Topologies...

Options:
  -f	overwrite existing log file
  --overhead <HORIZONTAL>,<VERTICAL> 
    	intra-layer and inter-layer additional wire length at each core
  --euclidean
    	use diagonal wiring in place of Manhattan wiring
  --seed <SEED>
    	random seed
  --host <D>,<R>
        the definition of host-sw wires

Dimension:
  <N>x<Z>	<Z> layers, <N> cores (square layout) on each layer

Topologies for each layer:
  mesh  	2-D mesh
  torus 	2-D torus
  ftorus	2-D folded torus
  hc    	hypercube
  htree 	H-tree
  cmesh 	C-mesh
  r-<D> 	random
  r-<D>-<R>	limited random
where
  <D>	degree
  <R>	max link distance in tiles

Example:
  #{$0} 16x4 mesh htree r-2 r-3-2
EOM

require "optparse"
require "pp"

def main
	$opts = Hash.new
	opt = OptionParser.new
	opt.on("-f") do |v|
		$opts[:overwrite] = v
	end
	opt.on("-q") do |v|
		$opts[:quiet] = v
	end
	opt.on("--overhead OUTER,INNER") do |v|
		$opts[:overhead] = v
	end
	opt.on("--euclidean") do |v|
		$opts[:euclidean] = v
	end
	opt.on("--seed SEED") do |v|
		$opts[:seed] = v
	end
	opt.on("--host D,R") do |v|
		$opts[:host] = v
	end
	opt.parse!(ARGV)
	if ARGV.length < 1
		puts $help_message
		exit(1)
	end
	topology = ARGV[0]
	dimension = topology.split("+")[0]
	parameter = ([topology] + ARGV[1..-1]).join("+")
	parameter += "(#{$opts[:seed]})" if $opts[:seed]
	parameter += "(h_#{$opts[:host]})" if $opts[:host]
	edgfn = "#{topology}.edges"
	mapfn = "#{dimension}.coord"
	outfn = "#{parameter}.edges"
	if File.exist?(outfn) && !$opts[:overwrite]
		STDERR.puts "Output file '#{outfn}' exists. Use -f to overwrite" unless $opts[:quiet]
		exit(0)
	end
	if $opts[:overhead]
		$outerhead = $opts[:overhead].scan(/\d+/)[0].to_i
		$innerhead = $opts[:overhead].scan(/\d+/)[1].to_i
	else
		$outerhead = 0
		$innerhead = 0
	end
	if $opts[:host]
		$hdeg = $opts[:host].scan(/\d+/)[0].to_i
		$hradius = $opts[:host].scan(/\d+/)[1].to_i
	end
	srand($opts[:seed].to_i)
	g = MyNetwork.new
	h = MyHost.new
	# ノードリストを生成
	dimensions = dimension.scan(/\d+/).collect{|x| x.to_i }
	g.make_layout(dimensions)
	# リンクを張る
	g.wire("zline", -1)
	ARGV[1..-1].each_with_index do |topo, layer|
		g.wire(topo, layer)
		if $opts[:host]
                  h.wire(g, layer)
                end
	end
	# エッジリストとマップを書き出す
	g.save_edgelist(outfn)
	g.save_layout(mapfn)
  if $opts[:host]
    h.save_edgelist(outfn, g)
  end
end

# 配列
class Array
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
	# 複数の配列の同じ位置にある要素同士を組み合わせてイテレート
	# 	[[1,2,3,4],[5,6,7],[8,9]].each_pair => [1,5,8],[2,6,9],[3,7,nil],[4,nil,nil]
	def each_pair
		self.max_by{|x| x.length }.each_index do |i|
			yield(self.collect{|x| x[i] })
		end
	end
	# ランダムマッチングを生成
	# 	ブロックにペア案が渡される。true を返せば採用、false を返せば不採用。
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

# 行列
# 行番号・列番号は0オリジン
class MyMatrix
	include Enumerable
	attr_reader :row_size, :col_size
	def initialize
		@data = Hash.new{|hash, key|
			hash[key] = Hash.new(0) # デフォルト値
		}
		@row_size = 0
		@col_size = 0
	end
	def initialize_copy(org)
		@data = Hash.new{|hash, key|
			hash[key] = Hash.new(0) # デフォルト値
		}
		org.each do |i, j, val|
			@data[i][j] = val
		end
	end
	def [](i, j)
		@data[i][j]
	end
	def []=(i, j, val)
# 		raise TypeError unless i.is_a?(Integer) && j.is_a?(Integer) && val.is_a?(Numeric) # 重い
		@data[i][j] = val
		@row_size = i + 1 if row_size < i + 1
		@col_size = j + 1 if col_size < j + 1
	end
	# スカラー和
	def +(other)
# 		raise TypeError unless other.is_a?(Numeric) # 重い
		a = self.class.new
		@row_size.times do |i|
			@col_size.times do |j|
				a[i, j] = @data[i][j] + other
			end
		end
		a
	end
	# スカラー積
	def *(other)
# 		raise TypeError unless other.is_a?(Numeric) # 重い
		a = self.class.new
		@data.each do |i, r|
			r.each do |j, val|
				a[i, j] = val * other
			end
		end
		a
	end
	def empty?
		@data.empty?
	end
	def each
		@data.each do |i, r|
			r.each do |j, val|
				yield(i, j, val)
			end
		end
	end
	def each_sorted
          @data.sort.each do |i, r|
            r.sort.each do|j, val|
              yield(i, j, val)
            end
          end
        end
	def along_row(i)
		@data[i].each do |j, val|
			yield(j, val)
		end
	end
	def along_col(j)
		@data.each do |i, r|
			yield(i, r[j])
		end
	end
	def to_s
		a = Array.new
		@row_size.times do |i|
			b = Array.new
			@col_size.times do |j|
				b.push(@data[i][j].to_s)
			end
			a.push(b.join(" "))
		end
		a.join("\n")
	end
end

# 無向グラフ
class MyGraph
	require "set"
	include Enumerable
	attr_reader :edgeset
	def initialize
		@edgeset = Set.new
		@adjmat = MyMatrix.new
	end
	def initialize_copy(org)
		@edgeset = org.edgeset.dup # 注意! SortedSet の .dup はシャローコピーだお
		@adjmat = org.adj_matrix.dup
	end
	def edges
		@edgeset.to_a
	end
	def edge_count
		@edgeset.size
	end
	def adj_matrix
		@adjmat
	end
	def each_edge
		@edgeset.each do |e|
			yield(e)
		end
	end
	alias :each :each_edge
	def each_edge_sorted
		@edgeset.sort.each do |e|
			yield(e)
		end
	end
	def has_edge?(i, j)
		e = (i < j) ? [i, j] : [j, i]
		@edgeset.include?(e)
	end
	def add_edge(i, j)
		e = (i < j) ? [i, j] : [j, i]
		@edgeset.add(e)
		@adjmat[e[0], e[1]] = @adjmat[e[1], e[0]] = 1 # 対称行列、重みなし
		self
	end
	def add_edges(arr)
		arr.each do |i, j|
			add_edge(i, j)
		end
		self
	end
	def delete_edge(i, j)
		e = (i < j) ? [i, j] : [j, i]
		@edgeset.delete(e)
		@adjmat[e[0], e[1]] = @adjmat[e[1], e[0]] = 0 # 対称行列、重みなし
		self
	end
	def delete_edges(arr)
		arr.each do |i, j|
			delete_edge(i, j)
		end
		self
	end
	# 隣接頂点リスト
	def adj_list(i)
		adjset = SortedSet.new
		@adjmat.along_row(i) do |j, w|
			adjset.add(j) if w != 0
		end
		adjset.to_a
	end
	# 頂点次数
	def degree(i)
		d = 0
		@adjmat.along_row(i) do |j, w|
			d += 1 if w != 0
		end
		d
	end
	# エッジリストを読み込む
	def load_edgelist(filename)
		open(filename, "r") do |f|
			f.each_line do |line|
				i, j, w = line.split # split by whitespaces
				next unless i && j
				i = i.to_i
				j = j.to_i
				add_edge(i, j)
			end
		end
	end
	# エッジリストを書き出す
	def save_edgelist(filename)
		open(filename, "w") do |f|
			each_edge_sorted do |i, j|
				f.printf("%d %d\n", i, j)
			end
		end
	end
end

# 座標つき無向グラフ
# 	ラックの奥行方向が X 軸、幅方向が Y 軸、ラック内が Z 軸。(alloc2.rb は X, Y が逆)
class MyMap < MyGraph
	def initialize
		super()
		@nodes = Array.new # 全ノードリスト
		@racks = Array.new # 全ラックリスト
		@nlayers = 0 # レイヤ数
		@xs = Array.new # X 座標リスト (ラックの奥行方向)
		@ys = Array.new # Y 座標リスト (ラックの幅方向)
		@nodes_on = Hash.new # @nodes_on[レイヤ番号] = ノードリスト
		@layer_of = Hash.new # @layer_of[ノード番号] = レイヤ番号
		@nodes_in = Hash.new # @nodes_in[ラック番号] = ノードリスト
		@rack_of  = Hash.new # @rack_of[ノード番号] = ラック番号
		@coord_of = Hash.new # @coord_of[ラック番号] = 座標
		@rack_at  = Hash.new # @rack_at[座標] = ラック番号
		@dist_max = 0 # 対角ラック間距離
		@cable_total = 0 # 総配線長
	end
        def get_nodes
          return @nodes
        end
	# レイアウトを生成
	def make_layout(params)
# 		STDERR.puts "<<<<<<<< make_layout (#{params}) >>>>>>>>"
		raise "Specify the number of nodes" unless params[0]
		raise "Specify the rack size" unless params[1]
		nrack = params[0].to_i # 1レイヤあたりのコア数
		@nlayers = params[1].to_i
		nnode = nrack * @nlayers
		nx = Math.sqrt(nrack).ceil  # 奥行方向のコア数
		ny = (nrack.to_f / nx).ceil # 幅方向のコア数
		@nodes = (0...nnode).to_a
		@racks = (0...nrack).to_a
		@nodes.each do |n|
			r = n % nrack
			z = n / nrack
			@nodes_in[r] ||= Array.new
			@nodes_in[r][z] = n
			@nodes_on[z] ||= Array.new
			@nodes_on[z][r] = n
			@rack_of[n] = r
			@layer_of[n] = z
		end
		@racks.each do |r|
			x = (r / ny) * CORE_SIZE
			y = (r % ny) * CORE_SIZE
			@coord_of[r] = [x, y]
			@rack_at[[x, y]] = r
			@xs.push(x)
			@ys.push(y)
		end
		@xs = @xs.uniq.sort
		@ys = @ys.uniq.sort
		calc_distances
	end
	# レイアウトを書き出す
	def save_layout(filename)
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
			each_edge_sorted do |n1, n2|
				f.printf("%d %d %d\n", n1, n2, cable_between(n1, n2))
			end
		end
	end
	# 距離計算
	def calc_distances
		@racks.each do |r|
			x = dist_between(@coord_of[0], @coord_of[r])
			@dist_max = x if @dist_max < x
		end
	end
	# 配線長
	def cable_between(n1, n2)
		cable_of(dist_between(@coord_of[@rack_of[n1]], @coord_of[@rack_of[n2]]))
	end
	# 距離 → 配線長
	def cable_of(dist)
		(dist == 0) ? $innerhead : $outerhead + dist
	end
	# 距離
	def dist_between(p, q)
		if $cabling_scheme == :euclidean
			Math.sqrt((p[0] - q[0]) ** 2 + (p[1] - q[1]) ** 2).round
		else
			(p[0] - q[0]).abs + (p[1] - q[1]).abs
		end
	end
	# リンク追加
	def add_link(n1, n2)
# 		STDERR.puts "#{n1}--#{n2}"
		n = edge_count
		add_edge(n1, n2)
		@cable_total += cable_between(n1, n2) if edge_count > n
	end
	def add_links(arr)
		arr.each do |n1, n2|
			add_link(n1, n2)
		end
	end
	# リンク削除
	def delete_link(n1, n2)
		n = edge_count
		delete_edge(n1, n2)
		@cable_total -= cable_between(n1, n2) if edge_count < n
	end
	def delete_links(arr)
		arr.each do |n1, n2|
			delete_link(n1, n2)
		end
	end
	# ラックを X 方向に串刺し
	def racks_along_x
		@ys.each do |y| 
			yield(@xs.collect{|x| @rack_at[[x, y]] }.compact)
		end
	end
	# ラックを Y 方向に串刺し
	def racks_along_y
		@xs.each do |x|
			yield(@ys.collect{|y| @rack_at[[x, y]] }.compact)
		end
	end
	# ノードを X 方向に串刺し
	def nodes_along_x_on(layer)
		racks_along_x do |rs|
			yield(rs.collect{|r| @nodes_in[r][layer] }.compact)
		end
	end
	# ノードを Y 方向に串刺し
	def nodes_along_y_on(layer)
		racks_along_y do |rs|
			yield(rs.collect{|r| @nodes_in[r][layer] }.compact)
		end
	end
	# ノードを Z 方向に串刺し
	def nodes_along_z
		@racks.each do |r|
			yield(@nodes_in[r])
		end
	end
	# 各ラック内の各ノードをサイクリックに取得する Enumerator
	# 	enum[ラック番号].next = 次に使うノード番号
	def enum_nodes_cyclic
		enum = Hash.new 
		@racks.each do |r|
			enum[r] = @nodes_in[r].cycle
		end
		enum
	end
	# ラックを頂点とするグラフ
	def graph_of_racks
		g = MyGraph.new
		self.edges.each do |n1, n2|
			g.add_edge(@rack_of[n1], @rack_of[n2])
		end
		g
	end
end

# ネットワーク
class MyNetwork < MyMap
	def initialize
		super()
	end
	# リンクを張る
	def wire(arg, layer)
		opts = arg.split(/-/)
		topo = opts.shift
		prms = opts.collect{|x| x.to_i }
		begin
			case topo
			when "zline"
				zline(prms)
			when "mesh"
				mesh(prms, :line, layer)
			when "torus"
				mesh(prms, :ring, layer)
			when "ftorus"
				mesh(prms, :fring, layer)
			when "htree"
				htree(prms, layer)
			when "cmesh"
				cmesh(prms, layer)
			when "hc"
				hcube(prms, layer)
			when "r"
				random(prms, layer)
			else
				raise "Topology '#{topo}' is not defined"
			end
		rescue RuntimeError
			STDERR.puts "wire('#{arg}'): #{$!}"
# 			STDERR.puts $@.collect{|s| "\tfrom #{s}" } # stack trace
			exit(1)
		end
# 		STDERR.print "'#{arg}' => cable #{@cable_total.to_f / 100}m\n"
	end
	# レイヤ間接続
	def zline(prms)
		raise "No parameter required" unless prms.length == 0
# 		STDERR.puts "<<<<<<<< ZLINE >>>>>>>>"
		nodes_along_z do |nz|
			link(nz, :line)
		end
	end
	# 2次元メッシュ系
	def mesh(prms, kind, layer)
		raise "No parameter required" unless prms.length == 0
# 		STDERR.puts "<<<<<<<< MESH (#{kind}, #{layer}) >>>>>>>>"
		nodes_along_y_on(layer) do |ny|
			link(ny, kind)
		end
		nodes_along_x_on(layer) do |nx|
			link(nx, kind)
		end
	end
	# ハイパーキューブ
	def hcube(prms, layer)
		raise "No parameter required" unless prms.length == 0
		deg = (Math.log(@racks.length) / Math.log(2)).ceil
# 		STDERR.puts "<<<<<<<< HC (#{deg}, #{layer}) >>>>>>>>"
		@cube_index = -1
		link(@nodes_on[layer], :linecube, [2] * deg)
	end
	# H-tree
	def htree(prms, layer)
		raise "No parameter required" unless prms.length == 0
# 		STDERR.puts "<<<<<<<< HTREE (#{layer}) >>>>>>>>"
		link(@nodes_on[layer], :htree, @ys.length)
	end
	# C-mesh
	def cmesh(prms, layer)
		raise "No parameter required" unless prms.length == 0
# 		STDERR.puts "<<<<<<<< HTREE (#{layer}) >>>>>>>>"
		link(@nodes_on[layer], :cmesh, @ys.length)
	end
	# ランダム
	# 	prms[0] = 次数
	# 	prms[1] = 範囲
	def random(prms, layer)
		raise "1 or 2 parameters required" unless prms.length >= 1 && prms.length <= 2
# 		STDERR.puts "<<<<<<<< RANDOM (#{prms[0]}, #{prms[1]}, #{layer}) >>>>>>>>"
		if prms[1]
			radius = prms[1] * CORE_SIZE
			link(@nodes_on[layer], :limitrandom, prms) do |n1, n2|
				dist_between(@coord_of[@rack_of[n1]], @coord_of[@rack_of[n2]]) <= radius
			end
		else
			link(@nodes_on[layer], :random, prms)
		end
	end
	
        # 距離取得
        def get_dist_between(n1, n2)
          return dist_between(@coord_of[@rack_of[n1]], @coord_of[@rack_of[n2]])
        end

#          dist_between(@coord_of[@rack_of[n1])
#        get_dist_between(@coord_of[@rack_of[n1]], @coord_of[@rack_of[n2]]) <= radius

	# ノード群をリンクする
	def link(nodes, kind, params = [])
		case kind
		when :line
			link_line(nodes, false)
		when :ring
			link_line(nodes, true)
		when :fring
			link_fring(nodes)
		when :full
			link_full(nodes)
		when :linecube
			link_cube(nodes, params, false)
		when :ringcube
			link_cube(nodes, params, true)
		when :uniform
			link_uniform(nodes, params[0])
		when :random
			link_random(nodes, params[0])
		when :limitrandom
			link_random(nodes, params[0]) do |n1, n2|
				yield(n1, n2)
			end
		when :htree
			link_htree(nodes, params)
		when :cmesh
			link_cmesh(nodes, params)
		end
	end
	# 線状／環状リンク
	def link_line(nodes, ring)
		nodes.each_cons(2) do |n1, n2|
			self.add_link(n1, n2)
		end
		self.add_link(nodes[0], nodes[-1]) if ring
	end
	# 折りたたみ環状リンク
	def link_fring(nodes)
		self.add_link(nodes[0], nodes[1]) 
		nodes.each_cons(3) do |n1, n2, n3|
			self.add_link(n1, n3)
		end
		self.add_link(nodes[-2], nodes[-1])
	end
	# 完全結合リンク
	def link_full(nodes)
		nodes.combination(2) do |n1, n2|
			self.add_link(n1, n2)
		end
	end
	# キューブリンク
	# 	sizes = 各次元方向のノード数
	def link_cube(nodes, sizes, ring)
		return @cube_index += 1 if sizes.empty?
		a = Array.new
		sizes[0].times do
			a.push(link_cube(nodes, sizes[1..-1], ring))
		end
		sizes[0].times do |k|
			next unless k > 0 || ring
			[[a[k - 1]].flatten, [a[k]].flatten].each_pair do |i, j|
				next unless nodes[i] && nodes[j]
				self.add_link(nodes[i], nodes[j])
			end
		end
		a
	end
	# 均一リンク (次数指定)
	# 	8頂点の例 (▲印のパターンが追加される)
	# 	span= 　　　 2　　　　　　3 　　　　 4　　　　5 　　 6　
	# 	　　　　━━━━━━　━━━━━　━━━━　━━━　━━
	# 	　 [0]　○　　　　　　○　　　　　○　　　　○　　　○　
	# 	　 [1]　│○　　　　　│○　　　　│○　　　│○　　│○
	# 	　 [2]　○│○　　　　││○　　　││○　　││○　││
	# 	　 [3]　　○│○　　　○││○　　│││○　│││　││
	# 	　 [4]　　　○│○　　　○││○　○│││　│││　││
	# 	　 [5]　　　　○│○　　　○││　　○││　○││　││
	# 	　 [6]　　　　　○│　　　　○│　　　○│　　○│　○│
	# 	　 [7]　　　　　　○　　　　　○　　　　○　　　○　　○
	# 	　　　　━━━━━━　━━━━━　━━━━　━━━　━━
	# 	deg=5:　　　 ▲ 　　　　　▲　　　　 ▲ 　　　▲　　 ▲ 
	# 	deg=4:　　　 ▲ 　　　　　▲　　　　 　 　　　▲　　 ▲ 
	# 	deg=3:　　　 　 　　　　　▲　　　　 ▲ 　　　　　　 ▲ 
	# 	deg=2:　　　 　 　　　　　▲　　　　 　 　　　▲　　 　 
	# 	deg=1:　　　 　 　　　　　　　　　　 ▲ 　　　　　　 　 
	def link_uniform(nodes, deg)
		raise "Too small degree (#{deg})" unless deg >= 0
		return if deg == 0
		deg.times do |d|
			span = ((nodes.length - 2).to_f * (d + 1) / (deg + 1)).round + 2
			nodes.each_cons(span) do |way|
				next unless way.first && way.last
				self.add_link(way.first, way.last)
			end
		end
	end
	# ランダムリンク (次数指定)
	def link_random1(nodes, deg)
		raise "Too small degree (#{deg})" unless deg >= 0
		return if deg == 0
		deg = nodes.length - 1 if deg >= nodes.length
# 		STDERR.print "#{nodes}"
		a1 = Array.new
		plan = self.dup
		deg.times do |d|
			a2 = nodes.random_matching{|n1, n2|
				if plan.has_edge?(n1, n2)
					false
				elsif block_given?
					yield(n1, n2)
				else
					true
				end
			}
			a1.concat(a2)
			plan.add_links(a2)
		end
# 		STDERR.puts " => #{a1.length} links added. #{a1} = #{a1.uniq.length}"
		self.add_links(a1)
	end
	# ランダムリンク (次数指定、再試行する)
	def link_random(nodes, deg)
		raise "Too small degree (#{deg})" unless deg >= 0
		return if deg == 0
		deg = nodes.length - 1 if deg >= nodes.length
# 		STDERR.print "#{nodes}"
		a0 = Array.new
		RAND_TRIAL.times do |t|
			a1 = Array.new
			plan = self.dup
			deg.times do |d|
				a2 = nodes.random_matching{|n1, n2|
					if plan.has_edge?(n1, n2)
						false
					elsif block_given?
						yield(n1, n2)
					else
						true
					end
				}
				a1.concat(a2)
				plan.add_links(a2)
			end
# 			print " #{a1.length}"
			a0 = a1 if a0.length < a1.length
			break if a0.length == nodes.length / 2 * deg
		end
# 		STDERR.puts " => #{a0.length} links added. #{a0} = #{a0.uniq.length}"
		self.add_links(a0)
	end
	# H-tree リンク
	# 	ny = Y方向に並ぶノード数
	def link_htree(nodes, ny)
		nbase = @nodes.length
		rbase = @racks.length
		knots = Array.new
		links_from = Hash.new
		imax = (ny.to_f / 2).ceil # Y方向に並ぶユニット数
          jmax = (nodes.length.to_f / ny.to_f / 2.0).ceil # X方向に並ぶユニット数
          STDERR.puts "imax: #{imax}"
          STDERR.puts "jmax: #{jmax}"
		nodes.each_with_index do |n, m|
			i = m % ny / 2 # ユニットのYインデックス
			j = m / ny / 2 # ユニットのXインデックス
			k = i + j * imax + nbase  # ユニット番号
 			STDERR.puts "[#{m}] #{n} #{j} #{i} #{k}"
			knots.push(k)
			links_from[k] ||= Array.new
			links_from[k].push([k, n])
		end
		knots = knots.uniq.sort
		knots.each_with_index do |k, l|
			r = l + rbase # ラック番号
			@nodes.push(k)
			@racks.push(r)
			@rack_of[k] = r
			@coord_of[r] = htree_locate_knot(links_from[k])
			# とりあえず @nodes_in, @nodes_on, @layer_of, @rack_at には反映しない
			self.add_links(links_from[k])
		end
		link_htree(knots, imax) if knots.length > 1
	end
	def htree_locate_knot(links)
		xs = Array.new
		ys = Array.new
		links.each do |k, n|
			x, y = @coord_of[@rack_of[n]]
			xs.push(x)
			ys.push(y)
		end
		[xs.uniq.avg.to_i, ys.uniq.avg.to_i] # とりあえず単純平均
	end
	# H-tree リンク
	# 	ny = Y方向に並ぶノード数
	def link_cmesh(nodes, ny)
		nbase = @nodes.length
		rbase = @racks.length
		knots = Array.new
		links_from = Hash.new
		imax = (ny.to_f / 2).ceil # Y方向に並ぶユニット数
                jmax = (nodes.length.to_f / ny.to_f / 2.0).ceil # X方向に並ぶユニット数
                STDERR.puts "imax: #{imax}"
                STDERR.puts "jmax: #{jmax}"
		nodes.each_with_index do |n, m|
			i = m % ny / 2 # ユニットのYインデックス
			j = m / ny / 2 # ユニットのXインデックス
			k = i + j * imax + nbase  # ユニット番号
 			STDERR.puts "[#{m}] #{n} #{j} #{i} #{k}"
			knots.push(k)
			links_from[k] ||= Array.new
			links_from[k].push([k, n])
		end
		knots = knots.uniq.sort
		knots.each_with_index do |k, l|
			r = l + rbase # ラック番号
			@nodes.push(k)
			@racks.push(r)
			@rack_of[k] = r
			@coord_of[r] = htree_locate_knot(links_from[k])
			# とりあえず @nodes_in, @nodes_on, @layer_of, @rack_at には反映しない
			self.add_links(links_from[k])
		end
          arr_along_x = Array.new(imax).map! {Array.new(jmax)}
          arr_along_y = Array.new(jmax).map! {Array.new(imax)}
          STDERR.puts "arr_along_x: #{arr_along_x}"
          STDERR.puts "arr_along_y: #{arr_along_y}"
          knots.each_with_index do |k, l|
            knots_i = l / jmax
            knots_j = l % jmax
            arr_along_x[knots_i][knots_j] = k
            arr_along_y[knots_j][knots_i] = k
          end
          arr_along_x.each do |elem|
            link_line(elem, false)
          end
          arr_along_y.each do |elem|
            link_line(elem, false)
          end
          STDERR.puts "arr_along_x: #{arr_along_x}"
          STDERR.puts "arr_along_y: #{arr_along_y}"
#		link_htree(knots, imax) if knots.length > 1
	end
end

class MyHost
  def initialize
    @edges = MyMatrix.new
  end

  # 1ホスト当たり1本ずつリンクを接続する
  def rhlink(sws, hosts, hps)
    RAND_TRIAL_HOST.times do |r|
      
      tmpsws = Array.new
      tmphosts = Array.new
      tmpedges = MyMatrix.new

      sws.each do |i|
        tmpsws << i
      end
      hosts.each do |i|
        tmphosts << i
      end
      @edges.each do |row, col, val|
        tmpedges[row, col] = val
      end

      tmpswconnects = Hash.new
      sws.each do |e|
        tmpswconnects[e] = hps
      end

      valid = true

      while 1 do
        break if tmphosts.empty?
        host0 = tmphosts.delete_at(rand(tmphosts.length))
        emptyconnects = Array.new
        sws.each do |i|
          emptyconnects << i if (tmpedges[host0, i] == 0 && tmpswconnects[i] > 0 && yield(host0, i))
        end

        if (!emptyconnects.empty?)
          sw0 = emptyconnects[rand(emptyconnects.length)]
          tmpsws.delete(sw0)
          tmpswconnects[sw0] -= 1
          tmpedges[host0, sw0] = 1
        else
          valid = false
          break
        end
      end

      if (valid)
        tmpedges.each do |row, col, val|
          @edges[row, col] = val
        end
        return
      else
        next
      end
    end

    raise "random matching error."
  end

  # host-sw間リンクの配線
  def wire(g, layer)
    hradius = $hradius * CORE_SIZE
    sws = Array.new
    g.nodes_along_x_on(layer) do |ny|
      sws.concat(ny)
    end
    sws = sws.sort

    sws.each do |s|
      @edges[s, s] = 1
    end

    ($hdeg - 1).times do |t|
      rhlink(sws, sws, 1) do |node0, node1|
        g.get_dist_between(node0, node1) <= hradius
      end
    end
  end

  # 配線長を重みとするエッジリスト(host-sw間)を書き出す
  def save_edgelist(filename, g)
    arr = Array.new
    arr = g.get_nodes
    arrmax = arr.max
    open(filename, "a") do |f|
      # @edges[host, sw] = length
      @edges.each_sorted do |i, j, val|
        f.printf("%d %d %d\n", j, i + arrmax + 1, g.get_dist_between(i, j)) if val == 1
      end
    end
  end

end

main if $0 == __FILE__
